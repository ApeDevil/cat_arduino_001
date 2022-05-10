
#include <Keyboard.h>
#include <Mouse.h>

#define safetyPin 21  //SAFETY: connect pin 21 to GND

const byte inputPins[] = {5,4,3,2,14,16,10};    //declaring inputs and outputs for buttonts
const byte outputPins[] = {9,8,7,6}; 
const byte inputPins_count = 7;
const byte outputPins_count = 4;

   
const byte buttonMap[4][7] = {{0,  4,    8,   99,   99,   23,   24},   
                              {1,  5,    9,   12,   20,   21,   22},
                              {2,  6,   10,   13,   17,   18,   19},
                              {3,  7,   11,   14,   99,   15,   16}};    // case 99 will never happen                         
bool EventRunning[36] = {false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false};
byte o;
byte i;
byte eventIndex;


const String catVersion = "CL-B00-BJ0-000";   // Serial Communication
String inBytes;
String eventSet[36];
String transientEvent; 
char eventComponent;

const char strEnder = 0xfa;
const char delimiter = 0xff;
const char mouseClickLeft = 0xfe;
const char mouseClickRight = 0xfc;
const char mouseClickMiddle = 0xfd;

int inBL;
int b;
int f;
byte eL;
byte eIndex;

int k;  //loop variable for subfunctions

const char ctrl = 0x80;
byte ctrl_pressed = 255;
const char shift = 0x81;
byte shift_pressed = 255;


const char w = 0x77;
byte w_pressed = 255;

const char a = 0x61;
byte a_pressed = 255;

const char d = 0x64;
byte d_pressed = 255;

const char s = 0x73;
byte s_pressed = 255;


#define pjx A1                      //joystick set up
#define pjy A0
int xVal;
int yVal;
int feh = 600;   // frirst event height
int fel = 400;   // frirst event low
int seh = 970;    //second event height
int sel = 50;    //second event low

bool JoystickPressed [8] = {false,false,false,false,
                               false,false,false,false};

String JoystickLayout [8] = {"xsel", "xfel", "xfeh", "xseh",
                             "ysel", "yfel", "yfeh", "yseh"};




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

  pinMode(pjx, INPUT_PULLUP);      // joysrick
  pinMode(pjy, INPUT_PULLUP);


  while (Serial.available() == 0)  {  Serial.println(catVersion);   delay(10);  }

  Keyboard.begin();
}





void loop() {

  ReadSerialcomm();

  ButtonRun();

  JoystickRun();
}





void ReadSerialcomm(){
  
  if (Serial.available() > 0){
    
      inBytes = Serial.readStringUntil(strEnder);
      delay(1000);
      
//      Serial.println(inBytes);

      inBL = inBytes.length();
      b=0;
      f=0;
      eIndex=0;
     
      
      for(b; b < inBL; b++) {  
          if (inBytes[b] == delimiter){
            
             
//              Serial.print(eIndex);
              
              eventSet[eIndex] = inBytes.substring(f,b);
//              Serial.print("  ");
//              Serial.println(eventSet[eIndex]);
              f = b+1;
              eIndex++;                    
           }       
      }    
  }
}




void ButtonRun(){  
  for (o=0; o<outputPins_count; o++){                  //looping through Outputpins and setting one at a time to LOW 
      digitalWrite(outputPins[o],LOW);   
      delayMicroseconds(5);
                
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
    delayMicroseconds(500);    
  }
}
void pressEventSet(byte index){
  
  if (EventRunning[index] == false){
    
    transientEvent = eventSet[index];
    eL = transientEvent.length();
    
    for(k=0; k < eL; k++){ 

        eventComponent = transientEvent[k];
      
        if(eventComponent == mouseClickLeft){            
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
            if(eventComponent == w){
              w_pressed = index;
            }
            if(eventComponent == s){
              s_pressed = index;           
            }
            if(eventComponent == a){
              a_pressed = index;
            }
            if(eventComponent == d){
              d_pressed = index;           
            }             
        }             
    }    
//    printButtonEvent(index);           // function for serial print of buttons
    EventRunning[index] = true;
  }
  else{
    //do nothing
//    printButtonEvent(index);  // function for serial print of buttons
  }
}
void releaseEventSet(byte index){

    transientEvent = eventSet[index];
    eL = transientEvent.length();
    
    for(k=0; k < eL; k++){ 

        eventComponent = transientEvent[k];
      
        if(eventComponent == mouseClickLeft){
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
        else if(eventComponent == w || eventComponent == s || eventComponent == a || eventComponent == d){
                if (index == w_pressed){
                  Keyboard.release(eventComponent);
                  w_pressed = 255;  
                }  
                else if (index == s_pressed){
                  Keyboard.release(eventComponent);
                  s_pressed = 255;  
                }     
                if (index == a_pressed){
                  Keyboard.release(eventComponent);
                  a_pressed = 255;  
                }  
                else if (index == d_pressed){
                  Keyboard.release(eventComponent);
                  d_pressed = 255;  
                } 
        }          
        else{     
            Keyboard.release(eventComponent);      
                     
        }         
    }
  
  EventRunning[index] = false;
}
void printButtonEvent(byte index){
  Serial.print("i ");
  Serial.print(index);
  Serial.print("\tpressed ");
  Serial.print(EventRunning[index]);
  Serial.println();
}




void JoystickRun(){
  xVal = analogRead(pjx);       //read the analog value, joystick
  yVal = analogRead(pjy);

//"xsel", "xfel", "xfeh", "xseh", "ysel", "yfel", "yfeh", "yseh"
//30      31      33      32       28     29      27      26


  if (xVal < fel && xVal > sel){  // x low value events
//    PressJoystickEvent(1);
    pressEventSet(31);
  }
  else{
//    ReleaseJoystickEvent(1);
    releaseEventSet(31);
  }
  
  if (xVal < sel){
//    PressJoystickEvent(0); 
    pressEventSet(30);   
  }
  else{
//    ReleaseJoystickEvent(0);
    releaseEventSet(30);
  }
  if (xVal > feh && xVal < seh){   // x heigh value events
//    PressJoystickEvent(2);
    pressEventSet(33);
  }
  else{
//    ReleaseJoystickEvent(2);
    releaseEventSet(33);
  }
  if (xVal > seh){
//    PressJoystickEvent(3);
    pressEventSet(32);
  }
  else{
//    ReleaseJoystickEvent(3);
    releaseEventSet(32);
  }


  if (yVal < fel && yVal > sel){   // y low value events
//    PressJoystickEvent(5);
    pressEventSet(29);
  }
  else{
//    ReleaseJoystickEvent(5);
    releaseEventSet(29);
  }
  
  if (yVal < sel){
//    PressJoystickEvent(4); 
    pressEventSet(28);   
  }
  else{
//    ReleaseJoystickEvent(4);
    releaseEventSet(28);
  }  
  if (yVal > feh && yVal < seh){   // y heigh value events
//    PressJoystickEvent(6);
    pressEventSet(27);
  }
  else{
//    ReleaseJoystickEvent(6);
    releaseEventSet(27);
  }
  if (yVal > seh){
//    PressJoystickEvent(7);
    pressEventSet(26);
  }
  else{
//    ReleaseJoystickEvent(7);
    releaseEventSet(26);
  }
}
  
