#include <JeeLib.h>
#include <util/parity.h>
#include "Arduino.h"

#define DEBUG

const long InternalReferenceVoltage = 1074L;  // Change this to the reading from your internal voltage reference

#define LED_PIN     9   // activity LED
#define BUTTON_PIN  8

unsigned char payload[] = "DB    ";

void startADC() {
    ADMUX = (0<<REFS1) | (0<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

    // Start a conversion
    ADCSRA |= _BV( ADSC );
}

int endADC() {
    // Wait for it to complete
    while( ( (ADCSRA & (1<<ADSC)) != 0 ) );

    // Scale the value
    return (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L;
}

void setup () {
#ifdef DEBUG
    Serial.begin(57600);
    Serial.print("init adc");
#endif
    startADC();
#ifdef DEBUG
    Serial.print("init rf12\n");
#endif
    rf12_initialize(2, RF12_868MHZ, 5);
    int value = endADC();
#ifdef DEBUG
    Serial.print("send start\n");
#endif
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, 0); // disable pull-up, since we have external pull down
    digitalWrite(LED_PIN, 1);
    *((int*)(payload + 4)) = value;
    //for (byte i = 0; i < 5; i++) {
#ifdef DEBUG
    //    Serial.print(i);
#endif
        rf12_sendStart(0, payload, sizeof payload);
#ifdef DEBUG
    Serial.print("waiting\n");
#endif
        while (!rf12_canSend()) {
        	rf12_recvDone();
        }
   // }
#ifdef DEBUG
    Serial.print("wait\n");
#endif
    digitalWrite(LED_PIN, 0);
#ifdef DEBUG
    Serial.print("sleep\n");
#endif
}

void loop() {
	Sleepy::loseSomeTime(10);
	//while (!digitalRead(BUTTON_PIN)) ;
    digitalWrite(LED_PIN, 1);
    //startADC();
    //int value = endADC();
    //*((int*)(payload + 4)) = value;
    //for (byte i = 0; i < 5; i++) {
#ifdef DEBUG
    //    Serial.print(i);
#endif
        rf12_sendStart(0, payload, sizeof payload);
#ifdef DEBUG
    //Serial.print("waiting\n");
#endif
        while (!rf12_canSend()) {
        	rf12_recvDone();
        }
    //}
    digitalWrite(LED_PIN, 0);
}


/*
void setup( void )
{
  Serial.begin( 57600 );
  Serial.println( "\r\n\r\n" );

  // REFS1 REFS0          --> 0 0 AREF, Internal Vref turned off
  // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)
  ADMUX = (0<<REFS1) | (0<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
}

void loop( void )
{
  int value;

  // Start a conversion
  ADCSRA |= _BV( ADSC );

  // Wait for it to complete
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) );

  // Scale the value
  value = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L;

  Serial.println( value );
  delay( 1000 );
}
*/

/*
void setup( void )
{
  Serial.begin( 57600 );
  Serial.println( "\r\n\r\n" );

  pinMode( LED_PIN, OUTPUT );
  digitalWrite( LED_PIN, LOW );
  delay( 1000 );

  analogReference( INTERNAL );
}

void loop( void )
{
  Serial.println( analogRead( 0 ) );
  digitalWrite( LED_PIN, HIGH );
  delay( 1000 );
}
*/

int main(void) {

  init();
  setup();

  while(true) {
    loop();
  }
}
