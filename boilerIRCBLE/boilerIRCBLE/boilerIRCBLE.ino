/*********************************************************************
This is an example for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

// This version uses call-backs on the event and RX so there's no data handling in the main loop!

#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include <Wire.h>

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 3
#define ADAFRUITBLE_RST 9

#define SLAVE_ADDRESS 0x60

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

int led = 7;
int debug = 1;

/**************************************************************************/
/*!
    This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("Advertising started"));
      break;
    case ACI_EVT_CONNECTED:
      Serial.println(F("Connected!"));
      break;
    case ACI_EVT_DISCONNECTED:
      Serial.println(F("Disconnected or advertising timed out"));
      break;
    default:
      break;
  }
}

/**************************************************************************/
/*!
    This function is called whenever data arrives on the RX channel
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{
  Serial.print(F("Received "));
  Serial.print(len);
  Serial.print(F(" bytes: "));
  for(int i=0; i<len; i++)
   Serial.print((char)buffer[i]); 

  Serial.print(F(" ["));

  for(int i=0; i<len; i++)
  {
    Serial.print(" 0x"); Serial.print((char)buffer[i], HEX); 
  }
  Serial.println(F(" ]"));

  /* Echo the same data back! */
  uart.write(buffer, len);
  
  /* Echo the same data over I2C to the BoilerMake Badge*/
  Serial.println("beginning transmission begin");
  Wire.beginTransmission(SLAVE_ADDRESS);
  Serial.println("beginning transmission end");
  //cycle through the whole buffer
  for(int i = 0; i < len; i++) {
    Serial.println(*buffer);
    Wire.write(*buffer);
    ++buffer;
  }
  Wire.write(0x07);  //end character - 'bell' character
  Serial.println("end transmission beginning");
  Wire.endTransmission(SLAVE_ADDRESS);
  Serial.println("end transmission end");
}

// function that executes whenever data is received from slave
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    if (debug) {
        Serial.print("rxEvent:");
        Serial.println(howMany);
    }
    while(Wire.available())
    {
        char c = Wire.read();
        if (debug) {
            Serial.print("i2c_rx:");
            Serial.println(c);
        } 
        else {
            Serial.print(c);
        }
    }
    digitalWrite(led, HIGH);
}

// function that executes whenever data is requested by slave
// this function is registered as an event, see setup()
void requestEvent() {
    if (debug) Serial.println("reqEvent");
    if (debug) digitalWrite(led, LOW);
    if (Serial.available()) {
        char inChar = '0'; 
        inChar = (char)Serial.read(); 
        Wire.write(inChar);
        if (debug) {
            Serial.print("sent:");
            Serial.println(inChar);
        }
    } else { //send some dummy data if no data is available from serial
        Wire.write("?");
        if (debug) Serial.println("!sent:");
    }
    digitalWrite(led, HIGH);
}



/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  pinMode(led, OUTPUT);     
  Wire.begin(4);                // join i2c bus with address #4
  Serial.begin(9600);
  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println("Adafruit Bluefruit Low Energy nRF8001 Callback Echo demo");
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  //Serial.begin(9600);           // start serial for input and output
  digitalWrite(led, HIGH);
  Serial.println("I2C SLAVE 5V");
  delay(2000); // the slave should become ready first
  Serial.println("Ready");

  uart.setRXcallback(rxCallback);
  uart.setACIcallback(aciCallback);
  // uart.setDeviceName("NEWNAME"); /* 7 characters max! */
  uart.begin();
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
  digitalWrite(led, LOW);
  Serial.println("LOOP");
  //check for updates from 
  //check for updates from Bluefruit
  uart.pollACI();
}
