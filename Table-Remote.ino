/*
Firmware designed to remotely operate tables with a 433MHz radio.
Designed to be deployed on an Adafruit Trinket M0.
More remotes can be spoofed by adding their transmission codes (in decimal) to the codes[] array
Transmission codes can be obtained by running the ReceiveDemo_Advanced example script
from the rc-switch library, and pressing the up or down button on the remote while pointed
towards the recieving antenna. More information available at https://github.com/sui77/rc-switch

The DotStar RGB LED onboard the Trinket M0 is controlled with the Adafruit Dotstar library,
available at https://github.com/adafruit/Adafruit_DotStar

Written by Fischer Moseley for CECFC during August 2018.
*/

#include <Adafruit_DotStar.h>
#include <RCSwitch.h>

#define BIT_LENGTH            24 //how many bits to send (used for binary zero padding)
#define WAIT_TIME             20 //time in ms to wait after a code is sent
#define UP_BTN_PIN            3  //pin the up button is connected to
#define DOWN_BTN_PIN          4  //pin the down button is connected to
#define TX_PIN                1  //pin the transmitter is connected to
#define DEBOUNCE_SETTLE_TIME  50 //how long the buttons have to be settled for (in ms)

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

RCSwitch mySwitch = RCSwitch(); //init the remote control
Adafruit_DotStar strip = Adafruit_DotStar(1, 7, 8, DOTSTAR_BRG); //init the RGB LED

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

  strip.begin(); //init DotStar
  strip.show();  //show the empty buffer

  mySwitch.enableTransmit(TX_PIN);
}

void raise_all(){
  Serial.println("raising...");
  strip.setPixelColor(0, 0xFF0000); //set DotStar to red
  strip.show(); //update the DotStar
  int len = sizeof(codes)/sizeof(codes[0]); //get length of array
  for(int i=0; i<len; i++){ //send each code
    mySwitch.send(codes[i], BIT_LENGTH);
    Serial.println(codes[i]);
    delay(WAIT_TIME); //give the hardware time to settle
  }
  strip.setPixelColor(0, 0x000000); //turn off the DotStar
  strip.show(); //update the DotStar
  Serial.println("raised");
}

void lower_all(){
  Serial.println("lowering...");
  strip.setPixelColor(0, 0x00FF00);//set DotStar to Blue
  strip.show(); //update the DotStar
  int len = sizeof(codes)/sizeof(codes[0]); //get length of array
  for(int i=0; i<len; i++){ //send each code
    mySwitch.send(codes[i]+1, BIT_LENGTH);
    Serial.println(codes[i]+1);
    delay(WAIT_TIME); //give the hardware time to settle
  }
  Serial.println("lowered");
  strip.setPixelColor(0, 0x000000); //turn off the DotStar
  strip.show(); //update the DotStar
}

void reset_states(){ //issues multiple lower commands to ensure desks are down
  Serial.println("resetting...");
  lower_all();
  delay(15000);
  lower_all();
  delay(15000);
  lower_all();
  Serial.println("reset");
} 

void read_state_execute(){ //read button state and send necessary commands
  if(UpButtonState&&DownButtonState){reset_states();} //reset the table states if both buttons are pressed
  else if(UpButtonState){raise_all();}                //if the up button is pressed then raise the tables
  else if(DownButtonState){lower_all();}              //if the down button is pressed then lower the tables
}

void loop() {
  if(Serial.available()){ //manual control over serial, also echos recieved characters
    char recieved = Serial.read();
    Serial.print("r: ");
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
