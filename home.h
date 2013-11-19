const int LED_PIN = 6;  // Port 3 digital
const int TEMP_PIN = 4; // Port 1 digital

unsigned char ringPayload[] = "DB   ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 1 = doorbell is ringing

unsigned char tempPayload[] = "DB       ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 3 = temperature measurement
//                                  ^^---- temperature, signed 16-bit int
//                                    ^^-- battery, centivolts


