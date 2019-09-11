/*
Firmware designed to remotely operate tables with a 433MHz radio.
Designed to be deployed on an Adafruit Trinket M0.
More remotes can be spoofed by adding their transmission codes (in decimal) to the codes[] array
Transmission codes can be obtained by running the ReceiveDemo_Advanced example script
from the rc-switch library, and pressing the up or down button on the remote while pointed
towards the recieving antenna. More information available at https://github.com/sui77/rc-switch

Written by Fischer Moseley for CECFC during August 2018.
Adapted by Branson Camp for CECFC during September 2019.
*/

#include <RCSwitch.h>

#define BIT_LENGTH            24 //how many bits to send (used for binary zero padding)
#define WAIT_TIME             20 //time in ms to wait after a code is sent
#define UP_BTN_PIN            3  //pin the up button is connected to
#define DOWN_BTN_PIN          4  //pin the down button is connected to
#define TX_PIN                10  //pin the transmitter is connected to
#define DEBOUNCE_SETTLE_TIME  50 //how long the buttons have to be settled for (in ms)

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

RCSwitch mySwitch = RCSwitch(); //init the remote control\

//button/debouncing variables
unsigned long UpLastDebounceTime=0;   //last time the up button was toggled
unsigned long DownLastDebounceTime=0; //last time the down button was toggled
bool UpButtonState = LOW;             //current state of the up button
bool DownButtonState = LOW;           //current state of the down button
bool LastUpButtonState = LOW;         //last state of the up button
bool LastDownButtonState = LOW;       //last state of the down button


//the codes array stores the up codes
//the down codes are equal to the up codes plus one
unsigned long int codes[]={ //stores the transmitter up button codes
  6532833, 
  13806609,
  10427409,
  15956497,
  78353,
  13108961,
  473617,
  6525457,
  252945,
  9861857,
  14031377,
  711697,
};

void setup() {
  Serial.begin(115200); //init serial

  //init I/O
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);

  mySwitch.enableTransmit(TX_PIN);
}

void raise_all(){
  Serial.println("RAISING...");
  int len = sizeof(codes)/sizeof(codes[0]); //get length of array
  for(int i=0; i<len; i++){ //send each code
    mySwitch.send(codes[i], BIT_LENGTH);
    Serial.print("SENDING CODE: ");
    Serial.println(codes[i]);
    delay(WAIT_TIME); //give the hardware time to settle
  }
  Serial.println("DONE RAISING");
}

void lower_all(){
  Serial.println("LOWERING...");
  int len = sizeof(codes)/sizeof(codes[0]); //get length of array
  for(int i=0; i<len; i++){ //send each code
    mySwitch.send(codes[i]+1, BIT_LENGTH);
    Serial.print("SENDING CODE: ");
    Serial.println(codes[i]+1);
    delay(WAIT_TIME); //give the hardware time to settle
  }
  Serial.println("DONE LOWERING");
}

void reset_states(){ //issues multiple lower commands to ensure desks are down
  Serial.println("RESETTING...");
  lower_all();
  delay(15000);
  lower_all();
  delay(15000);
  lower_all();
  Serial.println("DONE  RESETTING");
} 

void read_state_execute(){ //read button state and send necessary commands
  if(UpButtonState&&DownButtonState){reset_states();} //reset the table states if both buttons are pressed
  else if(UpButtonState){raise_all();}                //if the up button is pressed then raise the tables
  else if(DownButtonState){lower_all();}              //if the down button is pressed then lower the tables
}

void loop() {
  if(Serial.available()){ //manual control over serial, also echos recieved characters
    char recieved = tolower(Serial.read());
    Serial.print("[U]P, [D]OWN, OR [R]ESET: ");
    Serial.println(recieved);
    if(recieved=='u'){raise_all();}
    if(recieved=='d'){lower_all();}
    if(recieved=='r'){reset_states();}
  }

  //button debouncing code
  bool UpButtonReading = digitalRead(UP_BTN_PIN);
  bool DownButtonReading = digitalRead(DOWN_BTN_PIN);

  if(UpButtonReading!=LastUpButtonState){
    UpLastDebounceTime=millis();
  }
  if(DownButtonReading!=LastDownButtonState){
    DownLastDebounceTime=millis();
  }

  if((millis()-UpLastDebounceTime)>DEBOUNCE_SETTLE_TIME){
    if(UpButtonReading!=UpButtonState){
      UpButtonState=UpButtonReading;
      read_state_execute();
    }
  }

  if((millis()-DownLastDebounceTime)>DEBOUNCE_SETTLE_TIME){
    if(DownButtonReading!=DownButtonState){
      DownButtonState=DownButtonReading;
      read_state_execute();
    }
  }
  LastUpButtonState=UpButtonReading;
  LastDownButtonState=DownButtonReading;
}
