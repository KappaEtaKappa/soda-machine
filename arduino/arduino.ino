#include <SPI.h>
#include <Ethernet.h>
#include <stdio.h>
#include <stdlib.h>

#define ALLOWSODA 6

volatile unsigned long tagID = 0;
volatile unsigned long lastBitArrivalTime;
volatile int bitCount = 0;
int mode = 0;
int timeKeeper;

void ISRone(void)
{
  lastBitArrivalTime = millis();
  
  if(bitCount >= 14 && bitCount <=33){
  tagID <<= 1;
  tagID |= 1;
  }
  bitCount++;
}

void ISRzero(void)
{
  lastBitArrivalTime = millis();
  if(bitCount >= 14 && bitCount <=33){
  tagID <<= 1;
  }
  bitCount++;
  
}


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress server(10,0,0,10);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  
  pinMode(ALLOWSODA, OUTPUT); //allow soda
  digitalWrite(ALLOWSODA, LOW);
  
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);

  pinMode(2, INPUT);
  digitalWrite(2, HIGH);  // Enable pull-up resistor
  attachInterrupt(0, ISRzero, FALLING);

  pinMode(3, INPUT);
  digitalWrite(3, HIGH);  // Enable pull-up resistor
  attachInterrupt(1, ISRone,  FALLING);

  tagID = 0;
  bitCount = 0;
  Serial.print("pin setup done");
  timeKeeper = 0;
  
  delay(1000);
  startConnection();
  Serial.print("ethernet setup done");
}

int lines;
void loop()
{
//  Serial.println(bitCount);
  //  See if it has been more than 1/4 second since the last bit arrived
  if (mode == 0) {
    if(bitCount > 0 && millis() - lastBitArrivalTime >  250){
      timeKeeper = 0;
      Serial.print(bitCount, DEC);
      Serial.print(" bits: ");
      Serial.println(tagID);
      checkID(tagID);
      tagID = 0;
      bitCount = 0;
    }
  }
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (mode == 1) {
    if (client.available()) {
      char c = client.read();
      Serial.print(c);//debug
      if (c == '1') {
        validID();
      } else if (c == '0') {
        invalidID();
      } else {
        //bad data 
        Serial.println("Unknown character " + c);
        mode = 0;
      }
      // if the server's disconnected, stop the client:
      if (!client.connected()) {
        Serial.println("Disconnected");
        client.stop();
        mode = 0;
      }
    }
  }
  if(timeKeeper < 1023){
    timeKeeper++;
  }
  else {
    timeKeeper = 0;
    Serial.println("Timed Out. Resetting all the buffers.");
    clearAllBuffers();
  }
  delay(10);
}
void startConnection() {
   if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, server);
  }
  Serial.println("Connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 8124)) {
    Serial.println("Connected");
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("Connection failed");
    clearAllBuffers();
  } 
}
void checkID(unsigned long id) {
  mode = 1;
  lines = 0;
  timeKeeper = 0;
  
  client.println(id);
}
void invalidID() {
   Serial.println("Invalid ID");
   mode = 0;
}
void validID() {
  Serial.println("Valid ID");
  digitalWrite(ALLOWSODA, HIGH);
  delay(2000);
  digitalWrite(ALLOWSODA, LOW);
  mode = 0;
}

void clearAllBuffers(){
  bitCount = 0;
  tagID = 0;
  mode = 0;
}
// clears all interrupts
void clearinterrupts () {
  // the interrupt in the Atmel processor mises out the first negitave pulse as the inputs are already high,
  // so this gives a pulse to each reader input line to get the interrupts working properly.
  // Then clear out the reader variables.
  // The readers are open collector sitting normally at a one so this is OK

  for(int i = 2; i<4; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // enable internal pull up causing a one
    digitalWrite(i, LOW); // disable internal pull up causing zero and thus an interrupt
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // enable internal pull up
  }
  delay(10);
}

