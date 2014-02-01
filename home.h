const int LED_PIN = 6;  // Port 3 digital
const int TEMP_PIN = 4; // Port 1 digital

#define VCC_PIN A0      // Port 4 analog
#define VCC_MAX 620 // (4.0V / 2) / 3.3V * 1024   for NiMh
#define VCC_MIN 465 // (3.0V / 2) / 3.3V * 1024   for NiMh

unsigned char ringPayload[] = "DB   ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 1 = doorbell is ringing

unsigned char tempPayload[] = "DB       ";
//                               ^^------- recipient, always spaces for broadcast
//                                 ^------ 4 = temperature measurement 2.0
//                                  ^^---- temperature, signed 16-bit int
//                                    ^^-- battery, %


