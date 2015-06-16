#include <SPI.h>
#include <Ethernet.h>
#include <stdio.h>
#include <stdlib.h>

volatile unsigned long tagID = 0;
volatile unsigned long lastBitArrivalTime;
volatile int bitCount = 0;
int mode = 0;

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

  pinMode(2, INPUT);
  digitalWrite(2, HIGH);  // Enable pull-up resistor
  attachInterrupt(0, ISRzero, FALLING);

  pinMode(3, INPUT);
  digitalWrite(3, HIGH);  // Enable pull-up resistor
  attachInterrupt(1, ISRone,  FALLING);

  tagID = 0;
  bitCount = 0;
  Serial.print("setup done");

}

int lines;
void loop()
{
  //  See if it has been more than 1/4 second since the last bit arrived
  if (mode == 0) {
    if(bitCount > 0 && millis() - lastBitArrivalTime >  250){
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
//      Serial.print(c);//debug
      if (c == '\n') {
        lines++;
      } else if (lines == 8) { //after http boilerplate junk
        if (c == '1') {
          validID();
      client.stop();
        } else if (c == '0') {
          invalidID();
      client.stop();
        } else {
          lines++; //no more reading characters 
        }
      }
    }
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println("Disconnected");
      client.stop();
      mode = 0;
    }
  }
}

void checkID(unsigned long id) {
  mode = 1;
  lines = 0;
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, server);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("Connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("Connected");
    // Make a HTTP request:
    client.print("GET /scan/");
    client.print(id);
    client.println(" HTTP/1.1");
    client.println("Host: 10.0.0.10");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("Connection failed");
  }

}
void invalidID() {
   Serial.println("Invalid ID");
}
void validID() {
   Serial.println("Valid ID");
}

