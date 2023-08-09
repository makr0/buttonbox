#include "HardwareSerial.h"
#include "taskmessages.h"
#include "comm.h"

extern QueueHandle_t serialSendQueue;
extern QueueHandle_t ledbarDataQueue;
extern QueueHandle_t screenDataQueue;

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
    xTaskCreatePinnedToCore( serialSendFunction, "serialS", 10000, NULL, 0, NULL, 0);
}

void serialReadFunction( void * parameter) {
    uint8_t idx = 0;                            // Index for new data pointer
    uint16_t bufStartFrame;                     // Buffer Start Frame
    char *p;                                    // Pointer declaration for the new received data
    char incomingByte;
    char incomingBytePrev;
    uint16_t frameType = 0;
    ledMessage_struct messageToLED;
    screendata_struct messageToScreen;
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
        
        // Check if we reached the end of the Datapacket type
        if ( frameType == START_FRAME_LIGHTDATA && idx == sizeof(LightboxDataPacket) ) {
            uint16_t checksum;
            checksum = (uint16_t)(LightboxData.start ^ LightboxData.encoderPosition ^ LightboxData.encoderDirection);

            // we received a valid packet with Lightboxdata
            if (LightboxData.start == START_FRAME_LIGHTDATA && checksum == LightboxData.checksum) {
            //    messageToLED.speed = LightboxData.encoderPosition*100;
                messageToLED.brightness = LightboxData.encoderPosition;
                xQueueSend(ledbarDataQueue, (void*)&messageToLED, 0);
            }
            idx = 0;    // Reset index (it prevents to enter in this if condition in the next cycle)
            frameType = 0; // next frame can be different type
        }

        if ( frameType == START_FRAME_CARDATA && idx == sizeof(HovercarDataPacket) ) {
            uint16_t checksum;
            checksum   = (uint16_t)(carData.start ^ carData.cmd1 ^ carData.cmd2 ^ carData.speedR_meas ^ carData.speedL_meas 
                                           ^ carData.batVoltage ^ carData.boardTemp ^ carData.cmdLed);

            // we received a valid packet with cardata
            if (carData.start == START_FRAME_CARDATA && checksum == carData.checksum) {
                messageToScreen.speed = (carData.speedL_meas + carData.speedR_meas) / 2.0;
                messageToScreen.batVoltage = carData.batVoltage/100.0;
                messageToScreen.boardTemp = carData.boardTemp;
                xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
            }
            idx = 0;    // Reset index (it prevents to enter in this if condition in the next cycle)
            frameType = 0; // next frame can be different type
        }

        // Update previous states
        incomingBytePrev = incomingByte;
    }
}

void serialSendFunction(void* pvParameters) {
    buttondata_struct receivedMessage;
    while(1) {
        if (xQueueReceive(serialSendQueue, (void *)&receivedMessage, portMAX_DELAY /* Wait infinitely for new messages */) == pdTRUE) {
            ButtonData.start	        = (uint16_t)START_FRAME_BUTTONDATA;
            ButtonData.buttons          = receivedMessage.steeringButtons;
            ButtonData.mode             = receivedMessage.modeButton;
            ButtonData.checksum       = (uint16_t)(ButtonData.start ^ ButtonData.buttons ^ ButtonData.mode);
            LightboxPort.write((uint8_t *) &ButtonData, sizeof(ButtonData));
        }
    }
}
