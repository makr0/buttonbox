#include "HardwareSerial.h"
#include "taskmessages.h"
#include "comm.h"

extern QueueHandle_t serialSendLightdataQueue;
extern QueueHandle_t ledbarDataQueue;
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t ledMessageToScreenQueue;
extern QueueHandle_t encoderEventsQueue;

#define LIGHTBOX_RX_PIN 32
#define LIGHTBOX_TX_PIN 33

HardwareSerial LightboxPort(1);
void setupCommports() {
    Serial.begin(115200);
    LightboxPort.begin(115200,SERIAL_8N1,LIGHTBOX_RX_PIN,LIGHTBOX_TX_PIN);
    xTaskCreatePinnedToCore(
        serialReadFunction, /* Function to implement the task */
        "serialR", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        NULL,  /* Task handle. */
        0); /* Core where the task should run */
    xTaskCreatePinnedToCore( serialSendLightdataFunction, "serialS", 10000, NULL, 0, NULL, 0);
}

// receive from Lightdata or Cardata from Lightbox
void serialReadFunction( void * parameter) {
    uint8_t idx = 0;                            // Index for new data pointer
    uint16_t bufStartFrame;                     // Buffer Start Frame
    char *p;                                    // Pointer declaration for the new received data
    char incomingByte;
    char incomingBytePrev;
    uint16_t frameType = 0;
    ledMessage_struct messageToLED;
    ledMessage_struct ledMessageToScreen;
    screendata_struct messageToScreen;
    lightsMessage_struct messageToLightbox;
    encoderMessage_struct encoderMessage;
    while(1) {
        

        // Check for new data availability in the Serial buffer
        if (LightboxPort.available()) {
            incomingByte 	= LightboxPort.read();                                   // Read the incoming byte
            bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;       // Construct the start frame
        }
        else {
            continue;
        }

        // Copy received data
        if (bufStartFrame == START_FRAME_LIGHTDATA || bufStartFrame == START_FRAME_CARDATA) {
            if (bufStartFrame == START_FRAME_LIGHTDATA) {
                p       = (char *)&LightboxData;
                frameType = START_FRAME_LIGHTDATA;
            }
            if (bufStartFrame == START_FRAME_CARDATA) {
                p       = (char *)&carData;
                frameType = START_FRAME_CARDATA;
            }
            *p++    = incomingBytePrev;
            *p++    = incomingByte;
            idx     = 2;	
        } else if (idx >= 2) { // Save the new received data
            if( ( frameType == START_FRAME_LIGHTDATA && idx < sizeof(LightboxDataPacket) )
             || ( frameType == START_FRAME_CARDATA && idx < sizeof(HovercarDataPacket) )
            )
            {
                *p++    = incomingByte; 
                idx++;
            }
        }	
        
        // Check if we reached the end of the LightboxDataPacket
        if ( frameType == START_FRAME_LIGHTDATA && idx == sizeof(LightboxDataPacket) ) {
            uint16_t checksum;
            checksum = (uint16_t)(LightboxData.start ^ LightboxData.encoderPosition ^ LightboxData.encoderDirection);

            // we received a valid packet with Lightboxdata
            if (LightboxData.start == START_FRAME_LIGHTDATA && checksum == LightboxData.checksum) {
                encoderMessage.direction = LightboxData.encoderDirection;
                encoderMessage.encoderPosition = LightboxData.encoderPosition;
                xQueueSend(encoderEventsQueue, (void*)&encoderMessage, 0);

            //    messageToLED.speed = LightboxData.encoderPosition*100;
                messageToLED.brightness = LightboxData.encoderPosition;
                xQueueSend(ledbarDataQueue, (void*)&messageToLED, 0);
                ledMessageToScreen.brightness=messageToLED.brightness;
                xQueueSend(ledMessageToScreenQueue, (void*)&ledMessageToScreen, 0);
                messageToLightbox.command = LIGHTCOMMAND_BRIGHTNESS;
                messageToLightbox.value=LightboxData.encoderPosition;
                messageToLightbox.light = LIGHT_L;
                xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
                messageToLightbox.light = LIGHT_R;
                xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
                messageToLightbox.light = LIGHT_SEAT;
                xQueueSend(serialSendLightdataQueue, (void*)&messageToLightbox, 10);
            }
            idx = 0;    // Reset index (it prevents to enter in this if condition in the next cycle)
            frameType = 0; // next frame can be different type
        }

        // Check if we reached the end of the HovercarDataPacket
        if ( frameType == START_FRAME_CARDATA && idx == sizeof(HovercarDataPacket) ) {
            uint16_t checksum;
            checksum   = (uint16_t)(carData.start ^ carData.cmd1 ^ carData.cmd2 ^ carData.speedR_meas ^ carData.speedL_meas 
                                           ^ carData.batVoltage ^ carData.boardTemp ^ carData.currentDC ^ carData.drivingBackwards
                                           ^ carData.revolutions_l ^ carData.revolutions_r);

            // we received a valid packet with cardata
            if (carData.start == START_FRAME_CARDATA && checksum == carData.checksum) {
                messageToScreen.speed = (carData.speedL_meas - carData.speedR_meas) / 2.0  // average rpm, speeR is negative
                / 60.0 * 0.817 // m/s
                * 3.6; // km/h
                messageToScreen.batVoltage = carData.batVoltage/100.0;
                messageToScreen.boardTemp = carData.boardTemp;
                messageToScreen.cmd = carData.cmd1-carData.cmd2;
                messageToScreen.currentDC = carData.currentDC/100.0;
                messageToScreen.drivingBackwards = carData.drivingBackwards;
                messageToScreen.revolutions_l = carData.revolutions_l;
                messageToScreen.revolutions_r = carData.revolutions_r;
                xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
            }
            idx = 0;    // Reset index (it prevents to enter in this if condition in the next cycle)
            frameType = 0; // next frame can be different type
        }

        // Update previous states
        incomingBytePrev = incomingByte;
    }
}

void serialSendLightdataFunction(void* pvParameters) {
    lightsMessage_struct receivedMessage;
    while(1) {
        if (xQueueReceive(serialSendLightdataQueue, (void *)&receivedMessage, portMAX_DELAY /* Wait infinitely for new messages */) == pdTRUE) {
            LightsData.start	      = (uint16_t)START_FRAME_BUTTONDATA;
            LightsData.light          = receivedMessage.light;
            LightsData.command        = receivedMessage.command;
            LightsData.value          = receivedMessage.value;
            LightsData.checksum       = (uint16_t)(LightsData.start ^ LightsData.light ^ LightsData.command ^ LightsData.value);
            LightboxPort.write((uint8_t *) &LightsData, sizeof(LightsData));
        }
    }
}
