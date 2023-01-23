#include <SimpleFOC.h>

//phase pins
#define AH  PA8
#define AL PC13
#define BH  PA9
#define BL PA12
#define CH PA10
#define CL PB15
//backEMF pins
#define BEMFA  PA4
#define BEMFB  PC4
#define BEMFC PB11
#define COM    PB5

#define pi 3.1415

// BLDC driver instance     
BLDCDriver6PWM driver = BLDCDriver6PWM( PA8,PC13, PA9,PA12,PA10,PB15);
float input   =     0;//velocity input (potentiometer)
float i       =     0;
float p       =     1;//power ranging from 0 to 1
int   temp    = 50000;//temperatur sensor
float sinp    =     0;//sinus position 0-2Pi
int   c_print =     0;//counter serial print
int   stpc    =     0;//stepp counter

unsigned long currenttime = 0;
unsigned long timepast = 0;

bool  closed_loop = true;

int BEMF[600] = {0};
int BEMFc = 0;
String BEMFprint = "";

void setup() {
  Serial.begin(115200);
  pinMode( PC6,OUTPUT);//led
  pinMode(PC10, INPUT);//button
  
  pinMode( COM,OUTPUT);//COM
  digitalWrite(COM,HIGH);
  
  //init and enable driver 
  driver.init();//driver.enable(); // test if i can replace it with just driver.setPwm(0,0,0);
  sate(1,1,1);pwm(0,0,0);


 BEMF[0] = 0;
 BEMF[1] = 0;
}
void loop() {
 input=(float(analogRead(PB12))-511)/511;//potentiometer input,-512 to center;turning righ - negativ, left - positiv
 //Serial.print(input);
 temp =analogRead(PB14);
 if(temp<=580 && abs(input)>0.05){//check if temperature is low and input not near zero
 digitalWrite(PC6,HIGH);//led on
  if(abs(input)<0.3){
   closed_loop = true;
   sinp = sinp + (input/3);//input can be positiv and negativ (forward and reverse); /3 to limit stepp range to +-0.33, one sin is 6.28
   if(sinp > 2*pi){sinp=sinp-2*pi;}//if positiv; range keeping
   if(sinp <    0){sinp=2*pi+sinp;}//if negativ
   
   //+1 to be positiv; /2 to be in range 0-1; +pi/2 so its synchron with openloop 6stepp commmutation
   sate(1,1,1);
   pwm((sin(sinp+pi/2           )+1)/2*p, //add   0 degree
       (sin(sinp+pi/2+(2*pi/3)  )+1)/2*p, //add 120 degree
       (sin(sinp+pi/2+(2*pi/3*2))+1)/2*p);//add 240 degree
   _delay(1);

  }
  else{ 
   if(closed_loop == true){
    stpc = int(round(sinp/(2*pi)*5));//determin with which stepp to start wenn switching
    //input = 1; //just a boost to get into the closed loop, dont know if it helps
    closed_loop = false;
   }

   timepast = micros()-currenttime;
   currenttime = micros();
   Serial.print(String(timepast)+"\n");
   
   if(stpc>5){stpc=0;}
   if(stpc<0){stpc=5;}

   i = abs(input);
   switch(stpc){
    case 0:sate(1,1,0);pwm(i,0,0);msur(PB11);break;
    case 1:sate(0,1,1);pwm(0,0,i);msur( PA4);break;
    case 2:sate(1,0,1);pwm(0,0,i);msur( PC4);break;
    case 3:sate(1,1,0);pwm(0,i,0);msur(PB11);break;
    case 4:sate(0,1,1);pwm(0,i,0);msur( PA4);break;
    case 5:sate(1,0,1);pwm(i,0,0);msur( PC4);break;
    }
 
  if(stpc % 2 == 1){digitalWrite(COM,LOW);}
  if(stpc % 1 == 0){digitalWrite(COM,HIGH);}
  
   //BEMFprint = "";
   //for(int i=0;i<BEMFc;i=i+5){BEMFprint = BEMFprint + String(BEMF[i])+",";}
   //Serial.print("stepp:"+String(stpc)+"\n");
   //Serial.print(BEMFprint+"\n");

  }
 }
 else{
  digitalWrite(PC6,LOW);//led off
  sate(1,1,1);pwm(0,0,0);
 }
}

void  pwm(float a,float b,float c){_writeDutyCycle6PWM(a,b,c,driver.phase_state,driver.params);}
void sate(  int a,  int b,  int c){driver.setPhaseState((PhaseState)a,(PhaseState)b,(PhaseState)c);}
void msur(int pin){

 for(BEMFc=2;BEMFc<600;BEMFc++){//15
  //Serial.print(String(stpc*100)+","+String(analogRead(pin))+"\n");
  BEMF[BEMFc] = analogRead(pin);
  
  //BEMF will change when reversing, steps where back emf is zero will be max
  if(input > 0){
   if(stpc % 2 == 1 && BEMF[BEMFc-1] > BEMF[BEMFc]){stpc++;break;}
   if(stpc % 2 == 0 && BEMF[BEMFc-1] < BEMF[BEMFc]){stpc++;break;}
  }
  if(input < 0){
   if(stpc % 2 == 1 && BEMF[BEMFc-1] < BEMF[BEMFc]){stpc--;break;}
   if(stpc % 2 == 0 && BEMF[BEMFc-1] > BEMF[BEMFc]){stpc--;break;}
  }
 }
}

//void mA(){for(int i=0;i<13;i++){Serial.print("200,"+String(analogRead( PA4)));}}//measure phase A, common
//void mB(){for(int i=0;i<13;i++){Serial.print("400,"+String(analogRead( PC4)));}}//measure phase B, common
//void mC(){for(int i=0;i<13;i++){Serial.print("600,"+String(analogRead(PB11)));}}//measure phase C, common

 //serial print all states
 /*c_print++;
 if(c_print > 20){
  c_print = 0;
  s("tempi:");s( temp);s(" ");
  s("input:");s(input);s(" ");
  s("\n");
 }*/











