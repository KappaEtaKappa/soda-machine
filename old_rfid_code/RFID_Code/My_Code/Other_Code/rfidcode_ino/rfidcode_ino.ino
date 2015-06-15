#include <EEPROM.h>// Needed to write to EEPROM storage
#include<SD.h>//Needed to support SD card

#define BUTTON_PIN 5 //deletemode button
#define ALARM_PIN 6 // Alarm
#define DOOR_PIN 7 // Relay 
#define BLUE_LED 8 // Blue LED
#define RED_LED 9 // Red LED
#define GREEN_LED 10 // Grean LED
#define MAX_NUM_IDS 100 //maximum possible IDs stored
int buttonState = 0; 
//pins 11,12,13 are for the network/sd card
//pins 2 & 3 are for the reader

boolean programMode = false; // Initialize program mode to false
boolean deleteMode = false; // Initialize delete mode to false
boolean match = false; // initialize card match to false
//                        P P A A A A A A A A A A A A B B B B B B B B B B B B B B B B B B B B P
//                        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
boolean storedCard[35] = {
  1,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1}; // Stores an ID read from EEPROM
//                                                    0,-,4,-,-,2,-,-,4,-,-,2,-,-,2,-,-,4,-,- =0,424,224
unsigned long readCard; // Stores an ID read from the RFID reader
byte checksum = 0; // Stores the checksum to verify the ID 

int alarm = 0; // Extra Security, not used
int arraySize=100; // keeps track of the size of the array
int lastNum=-1; // keeps track of the position of the last value in the array
int fileswritten;
char basefile[16];
char currentfile[16];

unsigned long IDarray[MAX_NUM_IDS];

volatile int bit_count = 0;

// interrupt for reading in zero from reader
void DATA0(void) {
  if(bit_count >= 14 && bit_count <=33){ // we only care for ones in this range
    readCard=readCard << 1;
  }
  bit_count++;
}

// interrupt for reading in one from reader
void DATA1(void) {
  if(bit_count >= 14 && bit_count <=33){ // we only care for ones in this range
    readCard =(readCard << 1)+ 1;
  }
  bit_count++;
}

// setup the system 
void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  for (int i = 6;i<=10;i++){
    pinMode(i, OUTPUT);
  }
  basefile[0] = 'I';
  basefile[1] = 'D';
  basefile[2] = 'F';
  basefile[3] = 'i';
  basefile[4] = 'l';
  basefile[5] = 'e';
  basefile[6] = '\\';
  basfeile[7] = 'N';
  basefile[8] = 'u';
  basefile[9] = 'm';
  basefile[10] = 'b';
  basefile[11] = 'e';
  basefile[12] = 'r';
  basefile[13] = 's';
  basefile[14] = '0';
  basefile[15] = '\0';
  for (int i=0; i++; i<16){
    currentfile[i] = basefile[i];
  }
  // clear the interrupts
  clearinterrupts();
  // set up the interrupts
  attachInterrupt(0, DATA0, RISING);
  attachInterrupt(1, DATA1, RISING);
  delay(10); // wait
  
  if(!SD.begin(4)){ // start SD card
    Serial.println("Did not work");
    return;
  }
  // create the files, and open them
  if (!SD.exists("IDFile")){
    if(SD.mkdir("IDFile")){
      Serial.println("IDFile created");
    }
    else{
      Serial.println("IDFile was not created");
    }
  }
  else{
    Serial.println("IDFile already exists");
  }
  if (!SD.exists("IDFile\Numbers0"){
    File myfile = SD.open("IDFile\Numbers0");
    myfile.println(0);
    myfile.close();
  }
  if (SD.exists("IDFile\Numbers0"){
    File myfile = SD.open("IDFile\Numbers0");
    
  if (!SD.exists(~/ErrorFile)){
    if(SD.mkdir(~/ErrorFile)){
      Serial.println("ErrorFile created");
    }
    else{
      Serial.println("ErrorFile was not created");
    }
  }
  else{
    Serial.println("ErrorFile already exists");
  }
  
  digitalWrite(13, HIGH);  // show Arduino has finished initilisation
  Serial.println("READER_0001");
}

// main executable loop
void loop(){
  // if we have read in 35 bits, the number on the wiscard
  if(bit_count==35){
    if (deleteMode==true){ 
      // in delete mode, remove this id
      removeID(readCard,&lastNum,validID(readCard,lastNum));
      //printArray(lastNum);
      bit_count=0;
      readCard=0;
      deleteMode=false;
    }
    else{
      insertID(readCard, &lastNum);
      printArray(lastNum);
      bit_count=0;
      readCard=0;
    }

  }
  buttonState = digitalRead(BUTTON_PIN);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (deleteMode==false && buttonState == HIGH) {     
    // turn LED on:    
    deleteMode=true;
    Serial.println("Delete Mode activated!");
  } 
  else if(deleteMode==true && buttonState == LOW){
    // turn LED off:
    deleteMode=false;
    Serial.println("Delete Mode de-activated!");
  }
  deleteModeLights();
  delay(10);
}

void printArray (int last){
  for(int i=0;i<=last;i++){  
    Serial.print("IDarray: ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.println(IDarray[i]);
  }
}

void openDoor(){
  digitalWrite(8, HIGH);
  Serial.println("door has opened!!!");
  delay(5000);
  digitalWrite(8, LOW);
}
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

void insertID(unsigned long ID, int* last) {
  if (*last > MAX_NUM_IDS - 1){

  }
  int valid = validID(ID,*last);
  if (validID(ID,*last)>=0){
    return; 
  }
  *last = *last + 1;
  Serial.print("last");
  Serial.println(*last);
  IDarray[*last]=ID;
}

int validID(unsigned long ID, int last){
  for (int i = 0; i <= last; i++){
    if (ID==IDarray[i]) return i;
  }
  return -1;
}

void removeID(unsigned long ID,int *last,int pos){
  if (pos<0) return;
  for (int i =pos;i<*last;i++){
    IDarray[i]=IDarray[i+1];
  }
  (*last)--;
}

void deleteModeLights(){
  if (deleteMode == true){
    digitalWrite(BLUE_LED, HIGH);
    delay (100);
    digitalWrite(RED_LED, HIGH);
    delay (100);
    digitalWrite(BLUE_LED, LOW);
    delay (100);
    digitalWrite(RED_LED, LOW);
  }
}

void programModeLights(){
  if (programMode == true){
    digitalWrite(BLUE_LED, HIGH);
    delay (100);
    digitalWrite(GREEN_LED, HIGH);
    delay (100);
    digitalWrite(BLUE_LED, LOW);
    delay (100);
    digitalWrite(GREEN_LED, LOW);
  }
}

void writeToSD(int lastNum){
  File myFile;
  File toWrite;
  for (int fn = 8; fn<=0; fn--;)
  {
    currentfile[14] = pos + '0';
    if (SD.exists(currentfile)){
      myFile = SD.open(currentfile);
      currentfile[14] = pos + 1 + '0';
      SD.remove(currentfile);
      toWrite = SD.open(currentfile,FILE_WRITE);
      while (1)
      {
        data = myFile.read();
        if (data==-1)
        {
          toWrite.close();
          break;
        }
        else
        {
          toWrite.write(data);
        }
      }
    }
  }
  SD.remove(basefile);
  toWrite = SD.open(basefile);
  for (int i=0; i<size(IDArray); i++)
  {
    toWrite.println(IDArray[i]);
  }
  toWrite.close();
 }
 
 boolean readFromSD(){
   if (SD.exists(basefile))
   {
     File myFile = SD.open(basefile);
     char value = myfile.read();
     int count = 0;
     while (value != -1){
       if ((value <= '9') && (value >= '0'))
       {
         add = (int) value - '0';
         IDArray[count] = IDArray[count] * 10 + add;
       }
       value = myFile.read();
       if (value == '\n')
       {
         count++;
         value = myFile.read();
       }
     }
     myFile.close();
     return true;
   }
   return false;
 }
   
         


