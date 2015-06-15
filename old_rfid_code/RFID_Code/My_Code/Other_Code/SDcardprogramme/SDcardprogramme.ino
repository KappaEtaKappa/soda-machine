#include <SD.h>

#define SD_PIN 4
unsigned long IDArray[100];
File myfile;


void steup()
{
  IDArray[0]=34783;
IDArray[1]=23490;
IDArray[2]=32497;
    Serial.begin(9600);
    if(!SD.begin(SD_PIN)){
       Serial.println("SD does not register");
       return;
    }
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
  if (!SD.exists("IDFile/Numbers")){
    File myfile = SD.open("IDFile/Numbers",FILE_WRITE);
    myfile.print(0);
    myfile.close();
  }
  if (!SD.exists("ErrorFile")){
    if(SD.mkdir("ErrorFile")){
      Serial.println("ErrorFile created");
    }
    else{
      Serial.println("ErrorFile was not created");
    }
  }
  else{
    Serial.println("ErrorFile already exists");
  }   
}  

void loop ()
{
  
}

boolean writeToFile()
{
  char value;
  if (SD.exists("IDFile/Numbers")){
    File myfile = SD.open("IDFile/Numbers",FILE_READ);
    value = myfile.read();
    myfile.close();
    SD.remove("IDFile/Numbers");
    myfile = SD.open("IDFile/Numbers",FILE_WRITE);
    if (value == '5') value = '0';
    else value+=1;
    myfile.print(value);
    myfile.close();   
  }
  char file_p[10];
  char *file_path;
  file_path=file_p;
  file_path=
  
  file_path = file_path + value +'"';
  SD.remove(file_path);
  File myfile = SD.open(file_path);
  for (int i = 0; i < size(IDArray); i++){
    myfile.println(IDArray[i]);
  }
  myfile.close();
  return true;  
}

boolean readFromFile(){
  if (SD.exists("IDFile\Numbers")){
    File myfile = SD.open("IDFile/Numbers",FILE_READ);
    value = myfile.read();
    myfile.close();
    String file_path = "\"IDFile\\";
    file_path = file_path + value +'"';
    File myfile = SD.open(file_path);
    char value = myfile.read();
    while (value != -1){
    /*checks to make sure valid input */
      if ( ( value <= '9') && ( value >= '0')){ 
	add = (int) value - '0';
	IDArray[count] = IDArray[count] * 10;/* base ten left shift*/
	IDArray[count] = IDArray[count] + add;
      }
      value = myfile.read();
      if (value == '\n'){
        count++;
        value = myfile.read();
      }
    }
}
