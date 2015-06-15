/*
  Web client
 
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen
 
 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress server(10,0,0,28);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

}

int scan = 1;
int lines = 0;
int check = 0;
void loop()
{
  if (scan == 1) {
     checkID();
     scan = 0; 
  }
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    //Serial.print(c);//debug
    if (c == '\n') {
      lines++;
    } else if (lines == 8) { //after http boilerplate junk
      if (c == '1') {
        validID();
      } else if (c == '0') {
        invalidID();
      } else {
        lines++; //no more reading characters 
      }
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println("Disconnected");
    client.stop();
  
    // do nothing forevermore:
    while(true);
  }
}

void checkID() {

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
    client.println("GET /test HTTP/1.1");
    client.println("Host: 10.0.0.28");
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

