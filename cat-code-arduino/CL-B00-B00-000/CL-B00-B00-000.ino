
#include <Keyboard.h>
#include <Mouse.h>



#define safetyPin 21  //SAFETY: connect pin 21 to GND

const byte inputPins[] = {5,4,3,2,14,16,10};    //declaring inputs and outputs for buttonts
const byte outputPins[] = {9,8,7,6}; 
const byte inputPins_count = 7;
const byte outputPins_count = 4;

const byte buttonMap[4][7] = {{99,  8,    4,   0,   16,   15,   99},      // button layout for left side
                              {12,  9,    5,   1,   19,   18,   17},
                              {13,  10,   6,   2,   22,   21,   20},
                              {14,  11,   7,   3,   24,   23,   99}};     // case 99 will never happen
                              
bool EventRunning[36] = {false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false,false,false,false,false,
                          false,false,false,false,false,false};
byte o;
byte i;
byte eventIndex;

const String catVersion = "CL-B00-B00-000";   // Serial Communication
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
int a;
byte eL;
byte eIndex;

int k;  //loop variable for subfunctions

const char ctrl = 0x80;
byte ctrl_pressed = 255;

const char shift = 0x81;
byte shift_pressed = 255;
const char d = 0x64;









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


  Keyboard.begin();
  
  while (Serial.available() == 0)  {  Serial.println(catVersion);   delay(10);  }


}





void loop() {

  ReadSerialcomm();

  ButtonRun();

}





void ReadSerialcomm(){
  
  if (Serial.available() > 0){
    
      inBytes = Serial.readStringUntil(strEnder);
      delay(1000);
      
//      Serial.println(inBytes);

      inBL = inBytes.length();
      b=0;
      a=0;
      eIndex=0;
     
      
      for(b; b < inBL; b++) {  
          if (inBytes[b] == delimiter){
            
             
//              Serial.print(eIndex);
              
              eventSet[eIndex] = inBytes.substring(a,b);
//              Serial.print("  ");
//              Serial.println(eventSet[eIndex]);
              a = b+1;
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
//              Serial.println("ctrl pressed");
            }
            if(eventComponent == shift){
              shift_pressed = index; 
//              Serial.println("shift pressed");          
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
        else if(eventComponent == ctrl && index == ctrl_pressed){
            Keyboard.release(eventComponent);
            ctrl_pressed = 255;  
        }  
        else if(eventComponent == shift && index == shift_pressed){
            Keyboard.release(eventComponent);
            shift_pressed = 255;             
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
