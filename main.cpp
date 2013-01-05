#include <JeeLib.h>
#include <util/parity.h>
#include <avr/sleep.h>
#include "Arduino.h"

#define DEBUG

const long InternalReferenceVoltage = 1074L;  // Change this to the reading from your internal voltage reference

const int LED_PIN = 9;
const int BUTTON_INTERRUPT = 1;
const int BUTTON_PIN = BUTTON_INTERRUPT + 2;

unsigned char payload[] = "DB     ";
//                           ^^------- recipient, always spaces for broadcast
//                             ^------ 1 = doorbell is ringing
//                              ^^---- battery voltage, unsigned 16-bit int

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

void sendPacket() {
    digitalWrite(LED_PIN, 1);

#ifdef DEBUG
    Serial.println("start adc");
#endif
    startADC();
    endADC();
    startADC();

#ifdef DEBUG
    Serial.println("wake rf12");
#endif
	rf12_sleep(-1);

#ifdef DEBUG
    Serial.println("end adc");
#endif
    int value = endADC();
    payload[4] = 1;
    *((int*)(payload + 5)) = value;

#ifdef DEBUG
    Serial.print("send start, voltage: ");
    Serial.println(value);
#endif
    while (!rf12_canSend()) {
    	rf12_recvDone();
    }
    rf12_sendStart(0, payload, sizeof payload);
    rf12_sendWait(0);

#ifdef DEBUG
    Serial.println("sent");
#endif
    digitalWrite(LED_PIN, 0);
}

void setup () {
#ifdef DEBUG
    Serial.begin(57600);
    Serial.println("doorbell");
#endif
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, 0); // disable pull-up, since we have external pull up

    rf12_initialize(2, RF12_868MHZ, 5);
}

void buttonPressed() {
}

bool receivedAck() {
#ifdef DEBUG
	Serial.print("RF12: ");
	for (byte i = 0; i < rf12_len; ++i) {
		Serial.print(rf12_data[i]);
		Serial.print(" ");
	}
	Serial.println();
#endif
	return (rf12_len >= 5 && rf12_data[2] == 'D' && rf12_data[3] == 'B' && rf12_data[4] == 2);
}

MilliTimer retryTimer;

void loop() {
#ifdef DEBUG
    Serial.println("sleep");
    Serial.flush();
#endif
    rf12_sleep(0);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	attachInterrupt(1, buttonPressed, FALLING);
	sleep_mode();  //sleep now
	//--------------- ZZZZZZ sleeping here
	sleep_disable(); //fully awake now
	detachInterrupt(1);
#ifdef DEBUG
    Serial.println("wakeup");
#endif
	sendPacket();

	bool ack = false;
	int retries = 10;
	const int RETRY_INTERVAL = 300;
	retryTimer.set(RETRY_INTERVAL);
	while (!ack && retries > 0) {
		if (rf12_recvDone() && rf12_crc == 0 && receivedAck()) {
			ack = true;
		}
		if (retryTimer.poll()) {
			sendPacket();
			retries--;
			retryTimer.set(RETRY_INTERVAL);
		}
	}

	delay(500);
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
