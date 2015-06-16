//     Bit Explanation:   P P A A A A A A A A A A A A B B B B B B B B B B B B B B B B B B B B P
//     Bit Numbers:       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
//     Example:           1,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1 // Stores an ID read from EEPROM
//                        0,-,4,-,-,2,-,-,4,-,-,2,-,-,2,-,-,4,-,- =0,424,224

#include <EEPROM.h>// Needed to write to EEPROM storage
#include <SD.h>

char programModeCode[5] = "1924";
char deleteModeCode[5] = "2013";

//temporary
byte incomingByte;
uint16_t timeKeeper;

#define MAX_NUM_IDS 100 
//Arduino Hookups
int DELETE_MODE_LED = 9; // Yellow LED
int PROGRAM_MODE_LED = A5; // Purple LED
#define DOOR_PIN 8 // Relay 
#define SD_PIN 4; //SD select

//for the keypad
#define COLUMN_ONE 5
#define COLUMN_TWO 6
#define COLUMN_THREE 7
#define ROW_ONE A0
#define ROW_TWO A1
#define ROW_THREE A2
#define ROW_FOUR A3
char prevChar; //debouncing! woo!
char readChar;
char codeSequence[5];
short unsigned int codeLength = 0;

//bools
boolean noSD = true; // Initialize SD card not present flag to true
boolean programMode = false; // Initialize program mode to false
boolean deleteMode = false; // Initialize delete mode to false
boolean match = false; // initialize card match to false

unsigned long readCard; // Stores an ID read from the RFID reader
int lastNum=-1; // keeps track of the position of the last value in the array

unsigned long IDarray[MAX_NUM_IDS];
volatile int bitCount = 0;

// interrupt to read in zeros
void DATA0(void) {
  Serial.println("DATA0 interupt");
  if(bitCount >= 14 && bitCount <=33){
    readCard=readCard << 1;
  }
  bitCount++;
}

// interrupt to read in a one
void DATA1(void) {
  Serial.println("DATA1 interupt");
  if(bitCount >= 14 && bitCount <=33){
    readCard =(readCard << 1)+ 1;
  }
  bitCount++;
}

// on reset, this is run before the main loop
// this sets up the pins, serial communication, interrupts
// reads in from SD card all ids if there is a sd card
void setup()
{
  pinMode(COLUMN_ONE, OUTPUT);
  pinMode(COLUMN_TWO, OUTPUT);
  pinMode(COLUMN_THREE, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(PROGRAM_MODE_LED, OUTPUT);
  pinMode(DELETE_MODE_LED, OUTPUT);
  digitalWrite(PROGRAM_MODE_LED, LOW);
  digitalWrite(DELETE_MODE_LED, LOW);

  Serial.begin(9600);
  Serial.println("Starting");
  clearinterrupts();
  Serial.println("Starting2");
  attachInterrupt(0, DATA0, RISING);
  attachInterrupt(1, DATA1, RISING);
  Serial.println("Starting3");
  delay(10);
  Serial.println("Starting4");
//  //  Serial.println("READER_0001");
//  Serial.println("Trying to set-up SD card");
//
//  if(!SD.begin(4)){
//    Serial.println("Failed to set-up SD card");
//    noSD = true;
//    return;
//  }
//  else{
//    Serial.println("Successfully set-up SD card");
//  }
  bitCount = 0;
//  noSD = false;
//  readFromSD(IDarray, &lastNum);
//  Serial.print("lastNum = ");
//  Serial.println(lastNum);
//  printArray(lastNum);
}

//*****************************************************************
//*******************MAIN LOOP!!***********************************
//*****************************************************************
void loop(){
  // if we have read in all bits we need from a UW card
  Serial.println(bitCount);
  if(bitCount==35){
    timeKeeper = 0;
    if(readCard!=0){
      Serial.print("readCard = ");
      Serial.println(readCard);
    }
    timeKeeper = 0;
    // if we are adding this card
    if ( programMode == true ) {
      insertID(readCard, &lastNum);
      printArray(lastNum);
    } // if we are deleting this card
    else if ( deleteMode == true ) {
      removeID(readCard, &lastNum,validID(readCard,lastNum));
      printArray(lastNum);
    } // check if this is a valid card to open the door
    else {
      if ( validID(readCard, lastNum) >= 0 ) {
        openDoor(); 
      }
    }
    //clear the buffers.
    bitCount = 0;
    readCard = 0;
  }
//
//
//  //keypad logic
//  readChar = scanKeypad();
//  if(readChar != 'r' && readChar != 'n') {
//    timeKeeper = 0;
//    if (codeLength <= 3){
//      codeSequence[codeLength] = readChar;
//      codeLength++;
//    }
//    else{
//
//      for(int i = 0; i < codeLength; i++){
//        codeSequence[i] = 0; 
//      }
//      codeLength = 0;
//      codeSequence[codeLength] = readChar;
//      codeLength++;
//    }
//    if(codeLength == 4){
//      if(charCompare(codeSequence, programModeCode) && !deleteMode){
//        programMode = 1-programMode;
//        Serial.print("programMode = ");
//        Serial.println(programMode);
//      }
//      else if(charCompare(codeSequence, deleteModeCode) && !programMode){
//        deleteMode = 1-deleteMode;
//        Serial.print("deleteMode = ");
//        Serial.println((int)deleteMode);
//      }
//    }
//    Serial.print("keycode: ");
//    Serial.println(codeSequence);
//  }

  //can send commands from serial console!
  if (Serial.available() > 0) {
    timeKeeper = 0;
    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(incomingByte);
  
    switch (incomingByte) {
    case 'o':
      openDoor();
      break;
    case 'p':
      if(programMode == false && deleteMode == false){
        Serial.println("programMode engaged"); 
        programMode = true;
      }
      else{
        Serial.println("programMode disengaged");
        programMode = false;
      }
      break;
    case 'd':
      if(deleteMode == false && programMode == false){
        Serial.println("deleteMode engaged"); 
        deleteMode = true;
      }
      else{
        Serial.println("deleteMode disengaged");
        deleteMode = false;
      }
      break;
    }
  }

  //resets all the buffers if there are no inputs for awhile
  // this allows for correct functionality given a card that does not have 35 bits
  if(timeKeeper < 1023){
    timeKeeper++;
  }
  else {
    timeKeeper = 0;
    Serial.println("Timed Out. Resetting all the buffers.");
    clearAllBuffers();
  }
//  // this is SD card present checking. If a sd card is inserted then it will load
//  // the current ids onto it
//  if (noSD == true && SD.begin(4)) {
//    noSD = false;
//    writeToSD(lastNum); 
//  }
//  if (!SD.begin(4)) {
//    noSD = true;
//  }

  //for status LEDS
  digitalWrite(PROGRAM_MODE_LED, programMode);
  digitalWrite(DELETE_MODE_LED, (boolean)deleteMode);
  delay(10);
}


//****************************************************
//***********end of main loop, start of functions*****
//****************************************************
void clearAllBuffers(){
  //for the keypad
  codeLength = 0;
  //for the RFID Reader
  readCard = 0;
  bitCount = 0;
  //to return to normal mode
  programMode = false;
  deleteMode = false;
}


boolean charCompare(char firstChar[], char secondChar[]){
  String firstString(firstChar);
  String secondString(secondChar);
  if(firstString == secondString){
    return true;
  }
  return false;
}

// prints the contents of the id array
void printArray (int last){
  for(int i=0;i<=last;i++){  
    Serial.print("IDarray: ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.println(IDarray[i]);
  }
}

// this opens the door
void openDoor(){
  digitalWrite(8, HIGH);
  Serial.println("door has opened!!!");
  delay(5000);
  digitalWrite(8, LOW);
}


// inserts an id into to id array at the last position
// it updates the last number that is passed in
void insertID(unsigned long ID, int* last) {
  if (*last > MAX_NUM_IDS - 1){ // if we have too many, dont add
    return;
  }
  int valid = validID(ID,*last);
  if (validID(ID,*last)>=0){
    return; 
  }

  *last = *last + 1;
  Serial.print("last= ");
  Serial.println(*last);
  IDarray[*last]=ID;
  writeToSD(*last);
}

// checks if this is a valid id
// returns the position in the array
int validID(unsigned long ID, int last){
  for (int i = 0; i <= last; i++){
    if (ID==IDarray[i]) return i;
  }
  return -1;
}

//removes the id at the given position, updates the location of the last position
void removeID(unsigned long ID,int *last,int pos){
  if (pos<0) return;
  for (int i =pos;i<*last;i++){
    IDarray[i]=IDarray[i+1];
  }
  (*last)--;
  writeToSD(*last);
}

// overwrites all id's to a file
void writeToSD(int lastNum) {
  File toWrite;
  if (SD.exists("IDFile.txt")) {
    SD.remove("IDFile.txt");
  }
  toWrite = SD.open("IDFile.txt",FILE_WRITE);
  for (int i =0; i <= lastNum; i++) {
    toWrite.println(String(IDarray[i]));
  }
  toWrite.close();
}

// read all ids on a file
void readFromSD(long unsigned int IDArray[],int* total) {
  if (!SD.exists("IDFile.txt")) {
    return;
  }
  int lastNum = *total;
  int count = 0;
  IDArray[0] = 0;
  char value;

  File toRead = SD.open("IDFile.txt",FILE_READ);
  if(toRead){
    while (toRead.available()) {
      value = toRead.read();
      if (((int)value >= 48) && ((int)value <= 57)) { //48 is ascii for 0, 57 is ascii for 9
        int add = (int)value - 48;
        IDArray[count] = IDArray[count] * 10 + add;
      }
      if ((int)value == 10) { //10 is the ascii newline character
        count++;
        IDArray[count] = 0;
      }
    }
  }
  //clears the rest of the array
  for (int i = count; i < lastNum; i++) {
    IDArray[i] = 0;
  }
  *total = count - 1; //because we don't care about the last return in the IDFile.
  toRead.close();
}

//returns either the key being pressed, r for repeat, or n for no return
char scanKeypad(){
  char returnChar = 'n'; //n for no return
  digitalWrite(COLUMN_ONE, HIGH);
  if(analogRead(ROW_ONE) > 1000){
    returnChar = '1';
  }
  else if(analogRead(ROW_TWO) > 1000){
    returnChar = '4';
  }
  else if(analogRead(ROW_THREE) > 1000){
    returnChar = '7';
  }  
  else if(analogRead(ROW_FOUR) > 1000){
    returnChar = '*';
  }
  digitalWrite(COLUMN_ONE, LOW);

  digitalWrite(COLUMN_TWO, HIGH);
  if(analogRead(ROW_ONE) > 1000){
    returnChar = '2';
  }
  else if(analogRead(ROW_TWO) > 1000){
    returnChar = '5';
  }
  else if(analogRead(ROW_THREE) > 1000){
    returnChar = '8';
  }  
  else if(analogRead(ROW_FOUR) > 1000){
    returnChar = '0';
  }
  digitalWrite(COLUMN_TWO, LOW);

  digitalWrite(COLUMN_THREE, HIGH);
  if(analogRead(ROW_ONE) > 1000){
    returnChar = '3';
  }
  else if(analogRead(ROW_TWO) > 1000){
    returnChar = '6';
  }
  else if(analogRead(ROW_THREE) > 1000){
    returnChar = '9';
  }  
  else if(analogRead(ROW_FOUR) > 1000){
    returnChar = '#';
  }
  digitalWrite(COLUMN_THREE, LOW);

  if (returnChar == prevChar) {
    return 'r'; //r for repeat
  }
  else {
    prevChar = returnChar;
    return returnChar; 
  }
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




