
                             +------------+
                             | Hoverboard | ---> Motors, 2 Potis
                             +------------+
                               |  ^
            HovercarDataPacket |  |
                               |  | HovercarControlPacket
                               V  |
                             +----------+
                             |37,38     |   
                             |          |
             LED-Strips <--- | Lightbox | <--- Buttons, Rotary encoder
                             |          |
                             |32,33     |   
                             +----------+
                               |  ^
            LightboxDataPacket |  | ButtonboxDataPacket
            HovercarDataPacket |  | HovercarControlPacket
                               V  |
                             +-----------+
                             |32,33      |   
                             |           |
                Display <--- | Buttonbox | <--- 5 Buttons, 2 switches
                             |           |      5pos. dial-switch
                             +-----------+
