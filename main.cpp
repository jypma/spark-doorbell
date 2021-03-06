#include <JeeLib.h>
#include <util/parity.h>
#include <avr/sleep.h>
#include "Arduino.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//#define DEBUG

ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#include "home.h"

const int BUTTON_INTERRUPT = 1;
const int BUTTON_PIN = 3;
const int TEMP_INTERVAL = 60000;

volatile bool buttonPressed = false;

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress tempAddress;

unsigned int getSupplyVoltage() {
    unsigned int halfVoltage = analogRead(VCC_PIN);
#ifdef DEBUG
    Serial.print("VCC:");
    Serial.println(halfVoltage);
#endif
    halfVoltage = analogRead(VCC_PIN);
#ifdef DEBUG
    Serial.print("VCC:");
    Serial.println(halfVoltage);
#endif
    halfVoltage = analogRead(VCC_PIN);
#ifdef DEBUG
    Serial.print("VCC:");
    Serial.println(halfVoltage);
#endif
    if (halfVoltage > VCC_MAX) halfVoltage = VCC_MAX;
    if (halfVoltage < VCC_MIN) halfVoltage = VCC_MIN;
    unsigned int percentage = (halfVoltage - VCC_MIN) * 100 / (VCC_MAX - VCC_MIN);
#ifdef DEBUG
#endif
    return percentage;
}

void sendRingPacket() {
    digitalWrite(LED_PIN, 1);

#ifdef DEBUG
    Serial.println("wake rf12");
#endif
	rf12_sleep(RF12_WAKEUP);

    while (!rf12_canSend()) {
    	rf12_recvDone();
    }
    rf12_sendStart(0, ringPayload, sizeof ringPayload);
    rf12_sendWait(0);

#ifdef DEBUG
    Serial.println("sent");
#endif
    digitalWrite(LED_PIN, 0);
}

void sendTempPacket() {
#ifdef DEBUG
    digitalWrite(LED_PIN, 1);
#endif

#ifdef DEBUG
    Serial.println("get temp");
    Serial.flush();
#endif
    sensors.requestTemperatures();
    Sleepy::loseSomeTime(1000);

#ifdef DEBUG
    Serial.println("wake rf12");
#endif
	rf12_sleep(RF12_WAKEUP);

#ifdef DEBUG
    Serial.println("end adc");
#endif
    unsigned int voltage = getSupplyVoltage();
    *((unsigned int*)(tempPayload + 7)) = voltage;

    float tempC = sensors.getTempC(tempAddress);
    int temp = (int) (tempC * 100);
    *((int*)(tempPayload + 5)) = temp;

#ifdef DEBUG
    Serial.print("send start, voltage and temp: ");
    Serial.println(voltage);
    Serial.println(temp);
#endif
    while (!rf12_canSend()) {
    	rf12_recvDone();
    }
    rf12_sendStart(0, tempPayload, sizeof tempPayload);
    rf12_sendWait(0);

#ifdef DEBUG
    Serial.println("sent");
    digitalWrite(LED_PIN, 0);
#endif
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void onButtonPressed() {
	buttonPressed = true;
}

void setup () {
#ifdef DEBUG
    Serial.begin(57600);
    Serial.println("doorbell");
#endif
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, 0); // disable pull-up, since we have external pull up

    ringPayload[4] = 1;
    tempPayload[4] = 4;

    rf12_initialize(2, RF12_868MHZ, 5);

    sensors.begin();
    Serial.print("Found ");
    Serial.print(sensors.getDeviceCount());
    Serial.println(" temp sensor.");

    Serial.print("Parasite power is: ");
    if (sensors.isParasitePowerMode()) Serial.println("ON"); else Serial.println("OFF");

    if (!sensors.getAddress(tempAddress, 0)) Serial.println("Unable to find temp sensor");

    Serial.print("Device 0 Address: ");
    printAddress(tempAddress);
    Serial.println();

    sensors.setResolution(tempAddress, 12);
    sensors.setWaitForConversion(false);

	attachInterrupt(1, onButtonPressed, FALLING);
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
void handleDoorbell() {
	sendRingPacket();

	bool ack = false;
	int retries = 10;
	const int RETRY_INTERVAL = 300;
	retryTimer.set(RETRY_INTERVAL);
	while (!ack && retries > 0) {
		if (rf12_recvDone() && rf12_crc == 0 && receivedAck()) {
			ack = true;
		}
		if (retryTimer.poll()) {
			sendRingPacket();
			retries--;
			retryTimer.set(RETRY_INTERVAL);
		}
	}

	delay(500);
}

void loop() {
	sendTempPacket();

	buttonPressed = false;
	rf12_sleep(RF12_SLEEP);
	ADCSRA &= ~(1<<ADEN);
#ifdef DEBUG
    Serial.println("sleep");
    Serial.flush();
#endif
    Sleepy::loseSomeTime(TEMP_INTERVAL);
#ifdef DEBUG
    Serial.println("wakeup");
#endif
    ADCSRA |=  (1<<ADEN);
    if (buttonPressed) {
    	handleDoorbell();
    }
}

/*
3. Connect a 0.1 uF capacitor from AREF to ground
4. Connect power to the board
5. Upload the following Sketch...
6. Wait for a few readings to be displayed in Serial Monitor
7. Measure and record the voltage across the AREF capacitor.  In my case the voltage is 1.083.
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
