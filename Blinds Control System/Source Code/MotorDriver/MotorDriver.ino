#include <SoftwareSerial.h>
#include <DRV8835MotorShield.h>
#define Vcc 6
#define D1 5
#define sw1 3
#define sw2 2

bool ed = false, eu = false, state = true;
int Speed;
char command; 

DRV8835MotorShield motors;
SoftwareSerial BT(12, 11);                  //(TX,RX) bluetoothModule

void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(D1,OUTPUT);
  pinMode(sw1,INPUT_PULLUP);
  pinMode(sw2,INPUT_PULLUP);
  pinMode(Vcc,OUTPUT);
  digitalWrite(Vcc,HIGH);
  delay(1000);
}

void loop() {

  do                   
  {
    Drive(command);
    EndSwitch();
    digitalWrite(D1,LOW);
  }while(!BT.available());
  
  digitalWrite(D1,HIGH);
  char answer = BT.read();                 //read 1 byte command from master device
  
  if (answer == 'r' and state == true)     //if master send 'r' (request), motor send 'c'   
  { 
    BT.write('c');
    state = false;
  } 

  if (answer =='s' or answer =='d' or answer =='u')
  {command = answer;} 
   
  if (answer == 'e')
  {
    state = true;
    Restart(3000);
  }
}

void Restart(int delay_time){
  digitalWrite(Vcc,LOW);
  delay(delay_time);
  digitalWrite(Vcc,HIGH);
}
  
void Drive(char c){
  if (c == 's')
  {
   Stop();
  }
  if (c == 'u')
  {
    up(eu);
  }
  if (c == 'd')
  {
   down(ed);
  }
}

void Stop(){
  Speed = 0;
  motors.setM1Speed(Speed);
}

void up(bool endUp){
  if (endUp){ Stop(); }
  else{
  Speed = 200;
  motors.setM1Speed(Speed);}
}

void down(bool endDown){
  if (endDown){ Stop(); }
  else{
  Speed = -200;
  motors.setM1Speed(Speed);}
}

void EndSwitch(){

   if (digitalRead(sw1)== LOW)
   {
     delay(20);
     eu = true;
   } else { eu = false; }
   
   if (digitalRead(sw2)== LOW)
   {
     delay(20);
     ed = true;
   } else {ed = false; }
}
