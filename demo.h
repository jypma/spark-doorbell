const int LED_PIN = 4;  // Port 1 Digital
const int TEMP_PIN = 5; // Port 2 Digital

unsigned char ringPayload[] = "RB   ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 1 = doorbell is ringing

unsigned char tempPayload[] = "RB       ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 3 = temperature measurement
//                                  ^^---- temperature, signed 16-bit int
//                                    ^^-- battery, centivolts


