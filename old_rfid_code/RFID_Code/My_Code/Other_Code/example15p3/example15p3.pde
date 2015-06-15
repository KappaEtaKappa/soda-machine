/*
 Example 15.3
 read RFID tag, if it matches a preset tag, set a digital pin high for 10 seconds
 furthermore, write tag and time details to microSD card for later review
 tronixstuff.com/tutorials > Chapter 15
 */
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Wire.h>
#define DS1307_I2C_ADDRESS 0x68
// initialise variables
// first four required for SD card work
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;
// for RTC work
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))
int data1 = 0;
int sss=1;
int ok=-1;
// define the tag numbers that can have access
int yellowtag[14] = {
  2,51,67,48,48,67,69,49,48,68,53,51,55,3}; //  my yellow tag. Change this to suit your own tags, use example 15.1 sketch to read your tags
int redtag[14] = {
  2,51,67,48,48,67,67,51,69,69,51,50,68,3}; // my red tag...
int bosscard[14] = {
  2,51,69,48,48,49,65,57,56,57,70,50,51,3}; // my "supervisor" card, used to shut down system for microSD card removal
int newtag[14] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // used for read comparisons
int okdelay = 500; // this is the time the output will be set high for when an acceptable tag has been read
int notokdelay = 500; // time to show no entry (red LED)

void setup()
{
  Serial.begin(9600); // for debugging
  Serial.flush(); // need to flush serial buffer, otherwise first read from reset/power on may not be correct
  pinMode(3, OUTPUT); // this if for "rejected" LED
  pinMode(4, OUTPUT); // this will be set high when correct tag is read. Use to switch something on, for now - an LED. 
  Wire.begin(); // initialise i2c bus for DS1307 RTC

  // initialize the SD card
  if (!card.init()) error("card.init");
  // initialize a FAT volume
  if (!volume.init(card)) error("volume.init");
  // open the root directory
  if (!root.openRoot(volume)) error("openRoot");
  // create a new file. if the filename exists it will create a new different one
  char name[] = "WRITE00.TXT";
  for (uint8_t i = 0; i < 100; i++)
  {
    name[5] = i/10 + '0';
    name[6] = i%10 + '0';
    if (file.open(root, name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  if (!file.isOpen()) error ("file.create");
  // initial time values
  second = 0;
  minute = 36;
  hour = 17;
  dayOfWeek = 3;
  dayOfMonth = 18;
  month = 8;
  year = 10;
  // setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
}

void writeCRLF(SdFile &f)
{
  f.write((uint8_t *)"\r\n", 2);
}
void writeNumber(SdFile &f, uint32_t n)
{
  uint8_t buf[10];
  uint8_t i = 0;
  do {
    i++;
    buf[sizeof(buf) - i] = n%10 + '0';
    n /= 10;
  }
  while (n);
  f.write(&buf[sizeof(buf) - i], i);
}
void writeString(SdFile &f, char *str)
{
  uint8_t n;

  for (n = 0; str[n]; n++);
  f.write((uint8_t *)str, n);
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

void error_P(const char *str)
{
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode()) {
    PgmPrint("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  while(1);
}

boolean comparetag(int aa[14], int bb[14])
//  compares two arrrays, returns true if identical - good for comparing tags
{
  boolean ff=false;
  int fg=0;
  for (int cc=0; cc<14; cc++)
  {
    if (aa[cc]==bb[cc])
    {
      fg++;
    }
  }
  if (fg==14)
  {
    ff=true;
  }
  return ff;
}

void checkmytags()
//compares each tag against the tag just read
{
  ok=0; // this variable helps decision making, if it is 1, we have a match, zero - a read but no match, -1, no read attempt made
  if (comparetag(newtag,yellowtag)==true)
  {
    ok++;
  }
  if (comparetag(newtag,redtag)==true)
  {
    ok++;
  }
  if (comparetag(newtag,bosscard)==true)
  {
    bossMode();
  }
}

void readTag() 
// poll serial port to see if tag data is coming in (i.e. a read attempt)
{
  ok=-1;
  if (Serial.available() > 0) // if a read has been attempted
  {
    // read the incoming number on serial RX
    delay(100);  // Needed to allow time for the data to come in from the serial buffer. 
    for (int z=0; z<14; z++) // read the rest of the tag
    {
      data1=Serial.read();
      newtag[z]=data1;
    }
    Serial.flush(); // stops multiple reads
    // now to match tags up
    checkmytags(); // compare the number of the tag just read against my own tags' number
  }

  //now do something based on tag type
  if (ok>0) // if we had a match
  {
    digitalWrite(4, HIGH);
    getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    writeString(file, "----------------------------------------------------------------------------------------------");
    writeCRLF(file); // next line
    writeNumber(file, int(hour));
    writeString(file, ":");
    if (minute<10)
    {
      writeString(file, "0");
    }

    writeNumber(file, int(minute));
    writeString(file, ":");
    if (second<10)
    {
      writeString(file, "0");
    }

    writeNumber(file, int(second));
    writeString(file, " -- ");
    writeNumber(file, int(dayOfMonth));
    writeString(file, "/");
    if (month<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(month));
    writeString(file, "/20");
    writeNumber(file, int(year));
    writeString(file, " -- Tag # ");
    for (int z=0; z<14; z++) // read the rest of the tag
    {
      writeNumber(file, int(newtag[z]));
      newtag[z]=data1;
    }
    writeString(file, " Accepted ");
    writeCRLF(file); // next line
    delay(okdelay);
    digitalWrite(4, LOW);
    ok=-1;
  } 
  else if (ok==0) // if we didn't have a match
  {
    digitalWrite(3, HIGH);
    getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    writeString(file, "----------------------------------------------------------------------------------------------");
    writeCRLF(file); // next line
    writeNumber(file, int(hour));
    writeString(file, ":");
    if (minute<10)
    {
      writeString(file, "0");
    }

    writeNumber(file, int(minute));
    writeString(file, ":");
    if (second<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(second));
    writeString(file, " -- ");
    writeNumber(file, int(dayOfMonth));
    writeString(file, "/");
    if (month<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(month));
    writeString(file, "/20");
    writeNumber(file, int(year));
    writeString(file, " -- Tag # ");
    for (int z=0; z<14; z++) // read the rest of the tag
    {
      writeNumber(file, int(newtag[z]));
      newtag[z]=data1;
    }
    writeString(file, " Rejected ");
    writeCRLF(file); // next line
    delay(notokdelay);
    digitalWrite(3, LOW);
    ok=-1;
  }
}

void setDateDs1307(byte second,        // 0-59
byte minute,        // 0-59
byte hour,          // 1-23
byte dayOfWeek,     // 1-7
byte dayOfMonth,    // 1-28/29/30/31
byte month,         // 1-12
byte year)          // 0-99
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.send(0);
  Wire.send(decToBcd(second));    // 0 to bit 7 starts the clock
  Wire.send(decToBcd(minute));
  Wire.send(decToBcd(hour));      // If you want 12 hour am/pm you need to set
  // bit 6 (also need to change readDateDs1307)
  Wire.send(decToBcd(dayOfWeek));
  Wire.send(decToBcd(dayOfMonth));
  Wire.send(decToBcd(month));
  Wire.send(decToBcd(year));
  Wire.endTransmission();
}

// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.send(0);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.receive() & 0x7f);
  *minute     = bcdToDec(Wire.receive());
  *hour       = bcdToDec(Wire.receive() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.receive());
  *dayOfMonth = bcdToDec(Wire.receive());
  *month      = bcdToDec(Wire.receive());
  *year       = bcdToDec(Wire.receive());
}

void bossMode()
// if 'boss' tag is read, the data logging stops, the file is written to the microSD card, and the LEDs blink 
// when it is safe to remove the microSD card. 
{
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  writeString(file, "----------------------------------------------------------------------------------------------");
  writeCRLF(file); // next line
  writeCRLF(file); // next line
  writeNumber(file, int(hour));
  writeString(file, ":");
  writeNumber(file, int(minute));
  writeString(file, ":");
  if (second<10)
  {
    writeString(file, "0");
  }
  writeNumber(file, int(second));
  writeString(file, " -- ");
  writeNumber(file, int(dayOfMonth));
  writeString(file, "/");
  if (month<10)
  {
    writeString(file, "0");
  }
  writeNumber(file, int(month));
  writeString(file, "/20");
  writeNumber(file, int(year));
  writeString(file, " --");
  writeString(file, " Shutdown ");
  writeCRLF(file); // next line
  writeString(file, "----------------------------------------------------------------------------------------------");
  writeCRLF(file); // next line
  delay(500);
  file.close();
  delay(1000); // give the card writing time to wind up and finish
  // now just loop around forever blinking the LEDs
  while (data1!=12345)
  {
    digitalWrite(3, LOW);
    digitalWrite(4, HIGH);
    delay(750);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    delay(750);
  }
}

void loop()
{
  // writes startup time details, only once per reset cycle
  if (sss==1)
  {
    getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    writeString(file, "----------------------------------------------------------------------------------------------");
    writeCRLF(file); // next line
    writeNumber(file, int(hour));
    writeString(file, ":");
    if (minute<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(minute));
    writeString(file, ":");
    if (second<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(second));
    writeString(file, " -- ");
    writeNumber(file, int(dayOfMonth));
    writeString(file, "/");
    if (month<10)
    {
      writeString(file, "0");
    }
    writeNumber(file, int(month));
    writeString(file, "/20");
    writeNumber(file, int(year));
    writeString(file, " --");
    writeString(file, " Startup ");
    writeString(file, "----------------------------------------------------------------------------------------------");
    writeCRLF(file); // next line
    writeCRLF(file); // next line
    sss=0;
  }
  readTag(); // we should create a function to take care of reading tags, as later on 
  // we will want other things to happen while waiting for a tag read, such as 
  // displaying data on an LCD, etc
}






