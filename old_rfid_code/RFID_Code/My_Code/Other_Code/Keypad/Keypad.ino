int rowOne = A0;
int rowTwo = A1;
int rowThree = A2;
int rowFour = A3;
int columnOne = 4;
int columnTwo = 5;
int columnThree = 6;

void setup() {
  pinMode(columnOne, OUTPUT);
  pinMode(columnTwo, OUTPUT);
  pinMode(columnThree, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(columnOne, HIGH);
  switch(checkRows()){
  case 1: 
    Serial.println("1");
    break;
  case 2: 
    Serial.println("4");
    break;
  case 3: 
    Serial.println("7");
    break;
  case 4: 
    Serial.println("*");
    break;
  }
  digitalWrite(columnOne, LOW);

  digitalWrite(columnTwo, HIGH);
  switch(checkRows()){
  case 1: 
    Serial.println("2");
    break;
  case 2: 
    Serial.println("5");
    break;
  case 3: 
    Serial.println("8");
    break;
  case 4: 
    Serial.println("0");
    break;
  }
  digitalWrite(columnTwo, LOW);

  digitalWrite(columnThree, HIGH);
  switch(checkRows()){
  case 1: 
    Serial.println("3");
    break;
  case 2: 
    Serial.println("6");
    break;
  case 3: 
    Serial.println("9");
    break;
  case 4: 
    Serial.println("#");
    break;
  }
  digitalWrite(columnThree, LOW);
  delay(100);
}

int checkRows(){
  if(analogRead(rowOne)>=500){
    return 1;
  }
  if(analogRead(rowTwo)>=500){
    return 2;
  }
  if(analogRead(rowThree)>=500){
    return 3;
  }
  if(analogRead(rowFour)>=500){
    return 4;
  }
}



