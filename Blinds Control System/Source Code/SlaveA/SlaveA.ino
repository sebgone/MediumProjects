#include <SoftwareSerial.h>
#include <BH1750.h>                       //library for light sensor
#include <Wire.h>                         //library for I2C communication
#define D1 9                              //led indicating status of communication
#define Vcc 10  

BH1750 lightMeter;
SoftwareSerial BT(12, 11);                //(TX,RX) bluetoothModule

unsigned long start = 0, finish, elapsed;
boolean state = true;
char data[7] = {"a"};                     //char array to store light level value and point this is Salve A 
                                          //first element 'a' identifies what slave it is
char *p = data;

void setup() {
  Wire.begin();
  lightMeter.begin();
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(D1,OUTPUT);
  pinMode(Vcc,OUTPUT);
  digitalWrite(Vcc,HIGH);
  delay(1000);
}

void loop() {

  while(!BT.available())                  //measure light level until get request from master
  {
    int value = lightMeter.readLightLevel();
    itoa(value,p+1,10); 
    strcat(data,"e");                     //add "e" to the end of data frame
    digitalWrite(D1,LOW);
    delay(200); 
    Refresh(); 
  }
  digitalWrite(D1,HIGH);
  char answer = BT.read();                //read 1 byte command from master device
    
  if((answer=='r') and (state==true))     //if master send 'r' (request),send data 
  {
    state = false;
    BT.write(data);
  }
  if (answer == 'e'){
    start = millis();
    state = true;
    Restart(3000);
  }
}
void Restart(long delay_time){
  digitalWrite(Vcc,LOW);
  delay(delay_time);
  digitalWrite(Vcc,HIGH);
}
void Refresh(){
  finish = millis();
  elapsed = (finish - start)/1000;
  if(elapsed > 25)
  {
   Restart(40000);
   start = millis();
  }
}
