
#include <Keyboard.h>
#include <Mouse.h>
#include <EEPROM.h>

#include <adns5050.h>       //Add the ADNS5050 Library to the sketch: https://github.com/okhiroyuki/ADNS5050


#define safetyPin 21  //SAFETY: connect pin 21 to GND


ADNS5050 mouse(20,15,18);   //ADNS5050(int sdio, int sclk, int ncs);
#define NRESET 19
unsigned char value=0;    //This value will be used to store information from the mouse registers.
int x;
int y;
int uLast;
int vLast;



const byte SETTINGS_VERSION = 0xFA; // Schema version of settings in EEPROM

const byte inputPins[] = {5,4,3,2,14,16,10};    //declaring inputs and outputs for buttonts
const byte outputPins[] = {9,8,7,6}; 
const byte inputPins_count = 7;
const byte outputPins_count = 4;

const byte buttonMap[4][7] = {{0,  4,    8,   99,   99,   23,   24},    // button layout for right side
                              {1,  5,    9,   12,   20,   21,   22},
                              {2,  6,   10,   13,   17,   18,   19},
                              {3,  7,   11,   14,   99,   15,   16}};     // case 99 will never happen

bool EventRunning[36] = {false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false};
byte o;
byte i;
byte eventIndex;

const String catVersion = "CR-BW0-B00-M00";   // Serial Communication
String eventSet[36];
String transientEvent; 
char eventComponent;

const char strEnder = 0xfa;
const char delimiter = 0xff;
const char mouseClickLeft = 0xfe;
const char mouseClickRight = 0xfc;
const char mouseClickMiddle = 0xfd;

byte eL;

int k;  //loop variable for subfunctions

const char ctrl = 0x80;
byte ctrl_pressed = 255;
const char shift = 0x81;
byte shift_pressed = 255;



#define AInput 1      //pins for encoder
#define BInput 0
byte lastState = 0;   // variables for encoder 
byte steps = 0;
int  cw = 0;
byte AState = 0;
byte BState = 0;
byte State = 0;


void setup() {
  pinMode(safetyPin,INPUT);       //running the SAFETY-loop as long pin 21 is disconnected from GND
  digitalWrite(safetyPin,HIGH);
  while (digitalRead(safetyPin)){   
    Serial.println("safty loop");
    delay(5);
  }
  for(byte j=0; j<outputPins_count; j++){    //declaring the outputpins 
    pinMode(outputPins[j],OUTPUT);
    digitalWrite(outputPins[j],HIGH);  
  }
  for(byte j=0; j<inputPins_count; j++){     //declaring the inputpins
    pinMode(inputPins[j],INPUT_PULLUP);
  }

  pinMode(AInput, INPUT_PULLUP); // encoder
  pinMode(BInput, INPUT_PULLUP);


  pinMode(NRESET,OUTPUT);       //ADNS5050
  digitalWrite(NRESET,HIGH);
  mouse.begin();             //Initialize the ADNS5050
  delay(100);
  mouse.sync();    //A sync is performed to make sure the ADNS5050 is communicating

  Keyboard.begin();

  String settings = ReadEEPROMSettings();
  if(settings.length() == 0) {
    while (Serial.available() == 0)  {  Serial.println(catVersion);   delay(10);  }
  } else {
    parseSettings(settings);
  }
}


void loop() {
  ReadSerialcomm();

  ButtonRun();
}



void ReadSerialcomm(){
  if (Serial.available() > 0){
      String settings = Serial.readStringUntil(strEnder);
      delay(1000);
      parseSettings(settings);
      writeSettingsToEEPROM(settings);
  }
}


String ReadEEPROMSettings()
{
  if(EEPROM.read(0x00) != SETTINGS_VERSION) {
    return "";
  }
  
  int newStrLen = EEPROM.read(1);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(2 + i);
  }
  data[newStrLen] = '\0';
  return data;
}

void writeSettingsToEEPROM(String settings)
{
  byte len = settings.length();
  EEPROM.write(0, SETTINGS_VERSION); // Magic byte, if settings format changes change this.
  EEPROM.write(1, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(2 + i, settings[i]);
  }
}

void parseSettings(String settings) {
  int inBL = settings.length();
  int b = 0;
  int a = 0;
  byte eIndex = 0;
  for(b; b < inBL; b++) {  
    if (settings[b] == delimiter){
      eventSet[eIndex] = settings.substring(a,b);
      a = b+1;
      eIndex++;                    
      }
   }
}

void ButtonRun(){  
  for (o=0; o<outputPins_count; o++){                  //looping through Outputpins and setting one at a time to LOW 
      digitalWrite(outputPins[o],LOW);   
      delayMicroseconds(0);
      
      for(i=0; i<inputPins_count; i++){                // looping through Inputpins and checking for the LOW

          eventIndex = buttonMap[o][i];
          
          if(digitalRead(inputPins[i]) == LOW){
            pressEventSet(eventIndex);
          }      
          else {
            releaseEventSet(eventIndex);      
          }
      }      
    digitalWrite(outputPins[o],HIGH);              //setting the Outputpin back to HIGH 
//    delayMicroseconds(500);
    MouseRun();                       // ADNS5050 sub funktion
    EncoderRun();                         // encoder sub funktion
  }
}
void pressEventSet(byte index){
  
  if (EventRunning[index] == false){
    
    transientEvent = eventSet[index];
    eL = transientEvent.length();
    
    for(k=0; k < eL; k++){ 

        eventComponent = transientEvent[k];
      
        if(eventComponent == mouseClickLeft){
          Serial.println("LEFT MOUSE PRESSED");            
            Mouse.press(MOUSE_LEFT);
        }
        else if(eventComponent == mouseClickMiddle){
            Mouse.press(MOUSE_MIDDLE);
        }
        else if(eventComponent == mouseClickRight){
            Mouse.press(MOUSE_RIGHT);
        }
        else{
            Keyboard.press(eventComponent);
            if(eventComponent == ctrl){
              ctrl_pressed = index;
            }
            if(eventComponent == shift){
              shift_pressed = index;           
            }              
        }             
    }    
    //printButtonEvent(index);           // function for serial print of buttons
    EventRunning[index] = true;
  }
  else{
    //do nothing
    //printButtonEvent(index);  // function for serial print of buttons
  }
}
void releaseEventSet(byte index){
  if (EventRunning[index] == true){

    transientEvent = eventSet[index];
    eL = transientEvent.length();
    
    for(k=0; k < eL; k++){ 

        eventComponent = transientEvent[k];
      
        if(eventComponent == mouseClickLeft){
          Serial.println("LEFT MOUSE RELEASED");     
            Mouse.release(MOUSE_LEFT);
        }
        else if(eventComponent == mouseClickMiddle){
            Mouse.release(MOUSE_MIDDLE);            
        }
        else if(eventComponent == mouseClickRight){
            Mouse.release(MOUSE_RIGHT);      
        }
        else if(eventComponent == ctrl || eventComponent == shift){
                if (index == ctrl_pressed){
                  Keyboard.release(eventComponent);
                  ctrl_pressed = 255;  
                }  
                else if (index == shift_pressed){
                  Keyboard.release(eventComponent);
                  shift_pressed = 255;  
                }                      
        }          
        else{     
            Keyboard.release(eventComponent);      
                     
        }         
    }
      EventRunning[index] = false;
  }
}
void printButtonEvent(byte index){
  Serial.print("i ");
  Serial.print(index);
  Serial.print("\tpressed ");
  Serial.print(EventRunning[index]);
  Serial.println();
}




//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
void EncoderRun(){
  AState = digitalRead(AInput);
  BState = digitalRead(BInput) << 1;
  State = AState | BState;
  
  if (lastState != State){
    //EncoderPrint(State, cw, steps, AState, BState); // function for serial print
    
    switch (State) {
      case 0:
        if (lastState == 2){
          steps++;
          cw = 1;
        }
        else if(lastState == 1){
          steps--;
          cw = -1;
        }
        break;
      case 1:
        if (lastState == 0){
          steps++;
          cw = 1;
        }
        else if(lastState == 3){
          steps--;
          cw = -1;
        }
        break;
      case 2:
        if (lastState == 3){
          steps++;
          cw = 1;
        }
        else if(lastState == 0){
          steps--;
          cw = -1;
        }
        break;
      case 3:
        if (lastState == 1){
          steps++;
          cw = 1;
        }
        else if(lastState == 2){
          steps--;
          cw = -1;
        }
        break;
    }
  }
  lastState = State;
  Mouse.move(0, 0, cw);
  cw = 0;
}

void EncoderPrint(int State, int cw, int steps, int AState, int BState){
  Serial.print("State ");
  Serial.print(State);
  Serial.print("\t cw ");
  Serial.print(cw);
  Serial.print("\t steps ");
  Serial.print(steps);
  Serial.print("\t AState ");
  Serial.print(AState);
  Serial.print("\t BState ");
  Serial.println(BState);
}
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void MouseRun(){

    x = mouse.read(DELTA_X_REG);    //DELTA_X_REG store the x movements detected by the sensor
    y = mouse.read(DELTA_Y_REG);    //DELTA_Y_REG store the y movements detected by the sensor

    Mouse.move(x, -y, 0);

//    MousePrint(x,y);  
}

void MousePrint (int u, int v){
  if (u != uLast || v != vLast){
    Serial.print("x = ");       //DELTA_X_REG register
    Serial.print(u);
    Serial.print("   y = ");        //DELTA_Y_REG register 
    Serial.println(v);
    delay(10);
  }
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM