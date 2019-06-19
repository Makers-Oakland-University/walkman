/* NewWalkMan
*/
/*
      MCU Pins      1: GND                  4: --
    (1) (2) (3)     2: 5V from arduino      5: --
    (4) (5) (6)     3: Pin 0 - Arduino RX   6: Pin 1 - Arduino TX
    
                       /~\    head 11
                      |oo )   
  shoulderLeft 10     _\=/_   shoulderRight 12
        armLeft 9    /     \    armRight 13
                    //|/.\|\\   
                   ||  \_/  ||
      handLeft 8   || |\ /| ||  handRight 14
                    # \_ _/  #
        hipLeft  12   | | |     hipRight 21
        legLeft 16    | | |     legRight 17
        kneeLeft 15   []|[]     kneeRight 18
                      | | |
        footLeft 13   /_]_[_\  footRight 20
        ankleLeft 14           ankleRight 19   
*/

//------------------------------------------------
//Setup PS2 Controller
#include <PS2X_lib.h>

PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

#define dataPin 8
#define cmndPin 9
#define attPin 10
#define clockPin 12

//-------------------------------------------------
//Establish System Modes

int MODE = 1;
#define DEV 0
#define WALK 1
#define FLOSS 2

//number of modes
#define MODE_Num 3

//developer modes
int DevMODE = 0;
#define ARMS 0
#define ARM_R 1
#define ARM_L 2
#define LEG_R 3
#define LEG_L 4
#define HIP_HEAD 5

//number of developer modes
#define DevMODE_Num 6

//-------------------------------------------------
//Define servo pin values on the servo controller
// New servo values      //Old servo values 
#define footLeft 13       // 1 
#define footRight 20      
#define ankleLeft 14      // 2
#define ankleRight 19   
#define kneeLeft  15      // 3
#define kneeRight 18      
#define legLeft 16        // 4
#define legRight 17     
#define hipLeft 12        // 5
#define hipRight 21       // 16
#define shoulderLeft 11   // 10
#define shoulderRight 22  // 12
#define armLeft 9         
#define armRight 24       // 13
#define handLeft 10       // 8
#define handRight 23      // 14
#define head 11

#define commandDelay 1000

int Home[] = { head, 1500,
    footLeft,  1722,  ankleLeft,     1522,  kneeLeft,  1167,  legLeft,   1300,          
    footRight, 1767,  ankleRight,    1522,  kneeRight, 1300,  legRight,  1478,
    hipLeft,   1789,  shoulderLeft,  2200,  armLeft,   2300,  handLeft,  1500,
    hipRight,  1722,  shoulderRight, 800,  armRight,  700,  handRight, 1856,                        
  };

int SR = 1500;
int SL = 1500;
int AR = 1500;
int AL = 1500;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  error = ps2x.config_gamepad(clockPin, cmndPin, attPin, dataPin, true, true);

  if(error == 0){
    Serial.println("Found Controller, configuration successful");
  } else if (error == 1){
    Serial.println("No controller found, check wiring or ON/OFF switch");
  } else if (error == 2){
    Serial.println("Controller found but not accepting commands.");
  } else if (error == 3){
    Serial.println("Controller refusing to enter Pressures mode");
  }
  type = ps2x.readType();
  switch(type) {
       case 0:
        Serial.println("Unknown Controller type");
       break;
       case 1:
        Serial.println("DualShock Controller Found");
       break;
       case 2:
         Serial.println("GuitarHero Controller Found");
       break;
     }
  setHome(1000);
}

void loop() {
  if(ps2x.Button(PSB_START) && ps2x.Button(PSB_SELECT)){
    setHome(2000);
    exit;
  }

  if(ps2x.ButtonPressed(PSB_PAD_UP)){
    setHome(1000);
    MODE++;
    MODE = MODE % MODE_Num;
    displayMode(MODE, false);    
  }else if(ps2x.ButtonPressed(PSB_PAD_DOWN)){
    setHome(1000);
    MODE--;
    MODE = MODE % MODE_Num;
    displayMode(MODE, false);
  }
  if(MODE == DEV){
    if(ps2x.ButtonPressed(PSB_PAD_RIGHT)){      
      DevMODE++;
      DevMODE = DevMODE % DevMODE_Num;
      displayMode(DevMODE, true);
    } else if(ps2x.ButtonPressed(PSB_PAD_LEFT)){
      DevMODE--;
      DevMODE = DevMODE % DevMODE_Num;
      displayMode(DevMODE, true);
    }
  }
  
  if(MODE == DEV){
    if (DevMODE == ARMS){
      ps2x.read_gamepad(false, vibrate);
  
      AR = map(ps2x.Analog(PSS_RX), 140, 255, 500, 2500);
      AR = constrain(AR, 500, 2500);
      AL = map(ps2x.Analog(PSS_LX), 0, 120, 500, 2500);
      AL = constrain(AL, 500, 2500);
    
      SR = map(ps2x.Analog(PSS_RY), 0, 255, 500, 2500);
      SR = constrain(SR, 500, 2500);
      SL = map(ps2x.Analog(PSS_LY), 255, 0, 500, 2500);
      SL = constrain(SL, 500, 2500);
    
      int pos[] = {shoulderRight, SR, shoulderLeft, SL, armRight, AR, armLeft, AL};
      setBody(30, pos, sizeof(pos)/sizeof(int));
    }
  }
  else if (MODE == WALK){  
    walk();
  }
  else if (MODE == FLOSS) {
    floss();
  }
  //test();
  //setHome(250);
}

void test(){
  //  "#1P1722#2P1522#3P1167#4P1300#5P1789#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1522#20P1767T250");  
  //int setHome[] = {12, 2100};
  int wave[] = {shoulderRight, 1500, handRight, 1750};
  setHome(250);
  delay(1000);
  setBody(1000, wave, sizeof(wave)/sizeof(int));
  delay(1000);
}

void setBody(int rate, int command[], int count) {
  String string;
  String temp;
  int val;
  for(int i=0; i < count; i += 2){
    temp = "#" + (String)command[i];
    string.concat(temp);
    if(command[i+1] < 500){
      if(command[i] == 1){
        val = constrain(map(command[i+1], 0, 100, 2500, 500), 500, 2500);
      }else{
        val = constrain(map(command[i+1], 0, 100, 500, 2500), 500, 2500);
      }
      temp = "P" + (String)val;
    }
    else{temp = "P" + (String)command[i+1];}
    string.concat(temp);  
  }
  temp = "T" + (String)rate;
  string.concat(temp);
  Serial.println(string);
  delay(rate);
}

void setHome(int rate){
  setBody(rate, Home, sizeof(Home)/sizeof(int));
}

void walk(){
  int rate = 1000;

  int command[] = {footLeft, 1722, ankleLeft, 1522, kneeLeft, 1167, legLeft, 1300, hipLeft, 1789,
      handLeft, 1500, armLeft, 1522, shoulderLeft, 1300, head, 1500, shoulderRight, 2100,
      armRight, 1922, handRight, 1856, hipRight, 1772, legRight, 1478, kneeRight, 1300,
      ankleRight, 1522, footRight, 1767};
     
  setBody(rate, command, sizeof(command)/sizeof(int));
  
  int command1[] = {footLeft, 1522, ankleLeft, 1544, hipLeft, 1811, ankleRight, 1700, footRight, 1500};
  setBody(rate, command1, sizeof(command1)/sizeof(int));
  
  int command2[] = {legRight, 1167, ankleRight, 1922, footRight, 1633};
  setBody(rate, command2, sizeof(command2)/sizeof(int));
  
  int command3[] = {footLeft, 1678, ankleLeft, 1433, legLeft, 1256, hipLeft, 1767, hipRight, 1656, legRight, 1078, kneeRight, 1290, ankleRight, 1922, footRight, 1633};
  setBody(rate, command3, sizeof(command3)/sizeof(int));
  
  int command4[] = {footLeft, 2078, ankleLeft, 1433,  hipLeft, 1833, hipRight, 1611, legRight, 1233, ankleRight, 1722, footRight, 1936};
  setBody(rate, command4, sizeof(command4)/sizeof(int));
  
  int command5[] = {footLeft, 1967, ankleLeft, 1700, legLeft, 878, hipLeft, 1856, hipRight, 1633, legRight, 1300, ankleRight, 1678, footRight, 1944};
  setBody(rate, command5, sizeof(command5)/sizeof(int));
  
  int command6[] = {footLeft, 2100, ankleLeft, 1856, legLeft, 1122, hipRight, 1700, footRight, 1767};
  setBody(rate, command6, sizeof(command6)/sizeof(int));

  int command7[] = {footLeft, 1656, ankleLeft, 1544, legLeft, 1144, hipLeft, 1789, shoulderLeft, 1722, shoulderRight, 2056, hipRight, 1722, legRight, 1322, kneeRight, 1300, ankleRight, 1700};
  
  setBody(rate, command7, sizeof(command7)/sizeof(int));
  
  setBody(rate, command, sizeof(command)/sizeof(int));
  //setHome(1000);
/*  
  #1P1722#2P1522#3P1167#4P1300#5P1789#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1522#20P1767 T1000
  #1P1522#2P1544              #5P1811                                                                              #19P1700#20P1500 T1000
                                                                                                   #17P1167        #19P1922#20P1633 T1000
  #1P1678#2P1433       #4P1256#5P1767                                                      #16P1656#17P1078#18P1290#19P1944#20P1611 T1000
  #1P2078#2P1544              #5P1833                                                      #16P1611#17P1233        #19P1722#20P1936 T1000
  #1P1967#2P1700       #4P878 #5P1856                                                      #16P1633#17P1300        #19P1678#20P1944 T1000
  #1P2100#2P1856       #4P1122                                                             #16P1700                        #20P1767 T1000
  #1P1656#2P1544       #4P1144#5P1789              #10P1722        #12P2056                #16P1722#17P1322#18P1300#19P1700         T1000
*/
}

void floss(){
  boolean done = false;
  if(done){
    int rate = 1000;

    int start[] = {footLeft, 1722, ankleLeft, 1522, kneeLeft, 1167, legLeft, 1300, hipLeft, 1789,
        handLeft, 1500, armLeft, 1522, shoulderLeft, 1300, head, 1500, shoulderRight, 2100,
        armRight, 1922, handRight, 1856, hipRight, 1772, legRight, 1478, kneeRight, 1300,
        ankleRight, 1522, footRight, 1767};

    setBody(rate / 2, start, sizeof(start)/sizeof(int));

    int command[] = {footLeft, 1856, hipLeft, 1944, armLeft, 1211, shoulderLeft, 1122, shoulderRight, 2233, armRight, 1611, hipRight, 1789, legRight, 1478, footRight, 1833};
    setBody(rate / 2, command, sizeof(command)/sizeof(int));
    int command1[] = {footLeft, 1544, hipLeft, 1589, armLeft, 1811, shoulderLeft, 2011, shoulderRight, 2033, armRight, 1989, hipRight, 1522, legRight, 1522, footRight, 1522};
    setBody(rate, command1, sizeof(command1)/sizeof(int));
    int command2[] = {footLeft, 1858, hipLeft, 1944, armLeft, 1211, shoulderLeft, 1122, shoulderRight, 2233, armRight, 1611, hipRight, 1789, legRight, 1789, footRight, 1833};
    setBody(rate, command2, sizeof(command2)/sizeof(int));
    int command3[] = {footLeft, 1544, hipLeft, 1589, armLeft, 1967, shoulderLeft, 1122, shoulderRight, 2233, armRight, 2233, hipRight, 1522, legRight, 1522, footRight, 1522};
    setBody(rate, command3, sizeof(command3)/sizeof(int));
    int command4[] = {footLeft, 1856, hipLeft, 1944, armLeft, 1211, shoulderLeft, 1233, shoulderRight, 1611, armRight, 1589, hipRight, 1789, legRight, 1478, footRight, 1833};
    setBody(rate, command4, sizeof(command4)/sizeof(int));
    int command5[] = {footLeft, 1544, hipLeft, 1589, armLeft, 1967, shoulderLeft, 1122, shoulderRight, 2233, armRight, 2233, hipRight, 1522, legRight, 1522, footRight, 1522};
    setBody(rate, command5, sizeof(command5)/sizeof(int));
/*
  #1P1722#2P1522#3P1167#4P1300#5P1789#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1522#20P1767 T500
  #1P1856                     #5P1944       #9P1211#10P1122        #12P2233#13P1611        #16P1789                        #20P1833 T500
  #1P1544                     #5P1589       #9P1811#10P2011        #12P2033#13P1989        #16P1522#17P1522                #20P1522 T1000
  #1P1856                     #5P1944       #9P1211#10P1122        #12P2233#13P1611        #16P1789#17P1478                #20P1833 T1000
  #1P1544                     #5P1589       #9P1967                        #13P2233        #16P1522#17P1522                #20P1522 T1000
  #1P1856                     #5P1944       #9P1211#10P1233        #12P1611#13P1589        #16P1789#17P1478                #20P1833 T1000
  #1P1544                     #5P1589       #9P1967#10P1122        #12P2233#13P2233        #16P1522#17P1522                #20P1522 T1000
*/  
  }
}

void displayMode(int mode, boolean DevMODE){
  if(DevMODE){
    switch(mode){
      case 0:
        Serial.println("DevMODE set to ARMS");
        break;
      case 1:
        Serial.println("DevMODE set to ARM_R");
        break;
      case 2:
        Serial.println("DevMODE set to ARM_L");
        break;
      case 3:
        Serial.println("DevMODE set to LEG_R");
        break;
      case 4:
        Serial.println("DevMODE set to LEG_L");
        break;
      case 5:
        Serial.println("DevMODE set to HIP_HEAD");
        break;
    }
  }else{
    switch(mode){
      case 0:
        Serial.println("MODE set to DEV");
        break;
      case 1:
        Serial.println("MODE set to WALK");
        break;
      case 2:
        Serial.println("MODE set to FLOSS");
        break;
    }
  }
}


void walkOld() {
  //home
  Serial.println("#1P1722#2P1522#3P1167#4P1300#5P1789#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1522#20P1767T1000");
  delay(commandDelay);
  Serial.println("#1P1522#2P1544#3P1167#4P1300#5P1811#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1700#20P1500T1000");
  delay(commandDelay);
  Serial.println("#1P1522#2P1544#3P1167#4P1300#5P1811#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1167#18P1300#19P1922#20P1633T1000");
  delay(commandDelay);
  Serial.println("#1P1678#2P1433#3P1167#4P1256#5P1767#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1656#17P1078#18P1290#19P1944#20P1611T1000");
  delay(commandDelay);
  Serial.println("#1P2078#2P1544#3P1167#4P1256#5P1833#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1611#17P1233#18P1290#19P1722#20P1936T1000");
  delay(commandDelay);
  Serial.println("#1P1967#2P1700#3P1167#4P878#5P1856#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1633#17P1300#18P1290#19P1678#20P1944T1000");
  delay(commandDelay);
  Serial.println("#1P2100#2P1856#3P1167#4P1122#5P1856#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1700#17P1300#18P1290#19P1678#20P1767T1000");
  delay(commandDelay);
  Serial.println("#1P1656#2P1544#3P1167#4P1144#5P1789#8P1500#9P1522#10P1722#11P1500#12P2056#13P1922#14P1856#16P1722#17P1322#18P1300#19P1700#20P1767T1000");
  delay(commandDelay);
}

void flossOld() {
  int moveSpeed = 500;
  
  Serial.println("#1P1722#2P1522#3P1167#4P1300#5P1789#8P1500#9P1522#10P1300#11P1500#12P2100#13P1922#14P1856#16P1722#17P1478#18P1300#19P1522#20P1767T250");
  delay(moveSpeed / 2);
  Serial.println("#1P1856#2P1522#3P1167#4P1300#5P1944#8P1500#9P1211#10P1122#11P1500#12P2233#13P1611#14P1856#16P1789#17P1478#18P1300#19P1522#20P1833T250");
  delay(moveSpeed / 2);
  Serial.println("#1P1544#2P1522#3P1167#4P1300#5P1589#8P1500#9P1811#10P2011#11P1500#12P2033#13P1989#14P1856#16P1522#17P1522#18P1300#19P1522#20P1522T500");
  delay(moveSpeed);
  Serial.println("#1P1856#2P1522#3P1167#4P1300#5P1944#8P1500#9P1211#10P1122#11P1500#12P2233#13P1611#14P1856#16P1789#17P1478#18P1300#19P1522#20P1833T500");
  delay(moveSpeed);
  Serial.println("#1P1544#2P1522#3P1167#4P1300#5P1589#8P1500#9P1967#10P1122#11P1500#12P2233#13P2233#14P1856#16P1522#17P1522#18P1300#19P1522#20P1522T500");
  delay(moveSpeed);
  Serial.println("#1P1856#2P1522#3P1167#4P1300#5P1944#8P1500#9P1211#10P1233#11P1500#12P1611#13P1589#14P1856#16P1789#17P1478#18P1300#19P1522#20P1833T500");
  delay(moveSpeed);
  Serial.println("#1P1544#2P1522#3P1167#4P1300#5P1589#8P1500#9P1967#10P1122#11P1500#12P2233#13P2233#14P1856#16P1522#17P1522#18P1300#19P1522#20P1522T500");
  delay(moveSpeed);
}

/*
 
//range is 500 - 2500
//P
int leftX;
int leftY;
int rightX;
int rightY;

void setup() {
  Serial.begin(9600)
}

void loop() {
  Usb.Task();
  if (Xbox.XboxReceiverConnected) {
    if (Xbox.Xbox360Connected[0}) {
      leftX = Xbox.getAnalogHat(LeftHatX, 0);
      leftY = Xbox.getAnalogHat(LeftHatY, 0);
      rightX = Xbox.getAnalogHat(RightHatX, 0);
      rightY = Xbox.getAnalogHat(RightHatY, 0);

      tweakValues();

      if (leftX > 7500){
        moveWalkmanForward();
      }      
    }
  }
}


int mapToDegree(int pwmLength){
  map(pwmLength, 1, 180, 250, 2500);
}

void tweakValues(){
  //leftY = -leftY;
  //rightY = -rightY;

  leftX = constrain(leftX, -32767, 32767); // prevents overflow when abs() is called
  leftY = constrain(leftY, -32767, 32767); // prevents overflow when abs() is called
  rightX = constrain(rightX, -32767, 32767); // prevents overflow when abs() is called
  rightY = constrain(rightY, -32767, 32767); // prevents overflow when abs() is called
}

*/
 
