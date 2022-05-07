
#include <Wire.h>                                      //library i2C
#include <SPI.h>                                       //library SPI
#include <Adafruit_GFX.h>                              //library LCD PCD8544
#include <Adafruit_PCD8544.h>
#include <U8x8lib.h>                                   //library OLED SSD1306
#include <TimeLib.h>                                   //library to manage time 
#include <DS1307RTC.h>                                 //library RTC clock
#include <Servo.h>                                     //library to manage servos
#include <EEPROM.h>                                    //library EEPROM memory

#define SwLeft 9                                       //main screen letf button
#define SwRight 1                                      //main screen right button
#define SwEnter 0                                      //main screen center button

#define SCREEN1  1                                     //constansts identifying screen templates
#define SCREEN2  2
#define SCREEN3  3

#define FrontServo 0                                   //constants identifying front servo 0 and rear servo 1 
#define RearServo 1

#define MAX_RANGE 160                                  //extreme servos position
#define MIN_RANGE 10

#define F_SIZE  3                                      //const array size with front gears position
#define R_SIZE  8                                      //const array size with rear gears position

#define E_INIT 1023                                    //EEPROM memory block for factory settings 
#define analogSw A7                                    //analog pin controlling shifter buttons
                                                       //analog values for each button 
#define BTN1 990                                       //front - up
#define BTN2 520                                       //front - down
#define BTN3 350                                       //rear - up
#define BTN4 270                                       //rear - down

/*Screens*/
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 6, 7);  //init LCD and OLED         
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(U8X8_PIN_NONE);

/*Info on the screen*/
String str_mode;                                       //drive mode
String str_stats;                                      //measure state clock time "start" or "stop"
int bat;                                               //battery level

/*Variables state*/
bool long_press = false;                               //long button press flag
int mode = 1;                                          //drive mode vairable
int screen = 1;                                        //selected screen variable
bool  Exit_Configuration_Menu = false;                 //exit from configuration menu flag

/*Counters*/
time_t StopT, StartT, elap, mem_timer, sum_timer;      //variables to calculate time trip 
boolean state_timer = true;                            //activity time counter flag 
boolean start_timer = false;                           //current time trip counter state 
unsigned long start, finish, elapsed, check;           //long press and check battery variables

/*Cadence*/
volatile unsigned long crankTick;                      //hall sensor 1 interrupt detection 
volatile byte revCrank;                                //crank revolution
unsigned long timeCrank;                               //time between revolutions
float rpmCrank;                                        //revolution per minute

/*Wheel*/
float wheelDiameter = 660.00;                          //wheel diameter in mm
float wheelSpeed;                                      //drive speed
float distance = 0;                                    //trip distance 
float rpmWheel;                                        //wheel revolution per minute
volatile byte revWheel;                                //wheel revolutions
volatile unsigned long wheelTick;                      //hall sensor 1 interrupt detection 
unsigned long timeWheel;                               //time between revolutions

/*Average speed*/
float avg;

/*Gears*/
Servo servo[2];                                        //array of servo objects

int F[F_SIZE];                                         //array of front servo position values
int R[R_SIZE];                                         //array of rear servo position values

const int Front = FrontServo;                          //constant identifying front servo
const int Rear = RearServo;                            //constant identifying rear servo

int gearR = 1;                                         //current rear shift
int gearF = 1;                                         //current front shift 

/*Shifter screen*/
char sf[1];                                            //front gears in char form
char sr[1];                                            //rear gears in char form 
char sg[2];                                            //total speed in char form

/*Functions*/
void RPM()                                             //function to calculate cadence rpm
{
  /*If detect more than one crank revolution in main loop
  then calculate rpm based on time duration and revs amount.*/
  if(revCrank >= 1) 
  {
    detachInterrupt(1);
    rpmCrank =  60000.0 / (millis() - timeCrank) * revCrank;
    timeCrank = millis();
    revCrank = 0;
    attachInterrupt(1, crankCounter, RISING); 
  }
  /*If for 4 second there was any rev then reset rpm.*/
  else if(millis() - timeCrank > 4000)
  {
    rpmCrank = 0;
  }
}

void SPEED()                                           //function to calculate drive speed
{
  /*If detect more than four wheel revolutions 
  in main loop then calculate speed based on 
  time duration and revs amount.*/
  if(revWheel >= 4)
  {
    detachInterrupt(0);
    rpmWheel =  60000 / (millis() - timeWheel) * revWheel;
    timeWheel = millis();
    wheelSpeed = (rpmWheel * wheelDiameter * 3.141 * 60) / 1000000;
    revWheel = 0;
    attachInterrupt(0, wheelCounter, RISING); 
  }
  /*If for 4 second there was any rev then reset speed.*/
  else if(millis() - timeWheel > 4000)
  {
    wheelSpeed = 0;
  }
}

void wheelCounter()                                    //interrupt handler hall sensor 1 (wheel)
{ 
  /*Check if time duration from last interrupt is more than 80 ms.
  Sensitivity sensor filter. Count wheel revolutions and calculate
  distanse when user turn on cunter timer.*/
  if(millis() - wheelTick > 80)
  {
    revWheel++;
    if(start_timer)
      distance = distance + (wheelDiameter * 3.141 / 1000000);
  }
  wheelTick = millis();                          
}

void crankCounter()                                    //interrupt handler hall sensor 2 (crank)
{ 
  /*Check if time duration from last interrupt is more than 80 ms.
  Sensitivity sensor filter. Count crank revolution.*/
  if(millis() - crankTick > 80)
  {
    revCrank++;
  }
  crankTick = millis();                          
}

void Average()                                         //function to calculate average speed 
{
  /*If total time or distance equal zero then 
  average speed also.*/
  if(sum_timer == 0 or distance == 0)
    avg = 0;
  else if (start_timer == true)
   avg = (distance * 3600) / sum_timer; 
  /*If measure was activated then calculate average speed 
  based on distance and total time.*/
}

void StartTimer()                                      //function to start trip duration 
{
  if(state_timer)
  {
    StartT = RTC.get();
    state_timer = false;
  }
}

void StopTimer()                                       //function to stop trip duration counter
{
  if(!state_timer)
  {
    mem_timer = mem_timer + elap;
    state_timer = true;
  }
}
 
void displayTravelTime()                               //function to calculate information about time trip 
{
  if(start_timer == true)
  {
    StartTimer();
    StopT = RTC.get();
    elap = StopT - StartT;
    sum_timer = mem_timer + elap;
    digitalTimerDisplay(sum_timer);
  }
   
  if(start_timer == false)
  {
     StopTimer();
     digitalTimerDisplay(mem_timer);
  }
}

void digitalDate()                                     //function to display date
{
  printDate(day());
  printDate(month());
  display.print(year()-2000);
}

void printDate(int date)                               //function to format date 
{
   if(date < 10)
    display.print("0");
   display.print(date);
   display.print("/");
}

void digitalClockDisplay()                             //function to display current time 
{
  printDigits(hour());
  display.print(":");
  printDigits(minute());
  display.print(":");
  printDigits(second());  
}

void printDigits(int digits)                           //function to format current time 
{
  if(digits < 10)
    display.print("0");
  display.print(digits);
}

void digitalTimerDisplay(time_t t)                     //function to display tirp time    
{
  printDigitsTimer(hour(t));
  display.print(":");
  printDigitsTimer(minute(t));
  display.print(":");
  printDigitsTimer(second(t));  
}

void printDigitsTimer(time_t digits)                   //function to format trip time
{
  if(digits < 10)
    display.print("0");
  display.print(digits);
}

void displaySpeed(float value)                         //function to display speed 
{
  if(value < 10)
    display.print(F("  "));
  else if(value < 100)
   display.print(F(" "));
  display.print(value,0);
}

void displayRPM(float value)                           //function to display cadence 
{
  if(value < 10)
    display.print(F("  "));
  else if(value < 100)
   display.print(F(" "));
  display.print(value,0);
}

void checkbat()                                        //function to calculate battery voltage
{
  if(millis() - check > 4000)
  {
    int analog = analogRead(A0);
    float voltage = analog * (5.10/1024.0);
    bat = map(voltage*1000 ,3000.0 ,4050.0, 0, 100);
    if(bat > 100)
    {bat = 100;}
    if(bat < 0)
    {bat = 0;}
    check = millis();
  } 
}

void screen1()                                         //screen1 display battery level, date, time, drive mode 
{ 
    int p = 2;
    display.clearDisplay();
    display.setTextSize(1);
    
    display.setCursor(0,p);
    display.print(F("bat:  "));
    checkbat();
    display.print(bat);
    display.println(F(" %"));
    display.drawLine(0,p+=9,display.width(),p,BLACK);
        
    display.setCursor(0,p+=3);
    display.print(F("date: "));
    digitalDate();
    display.drawLine(0,p+=9,display.width(),p,WHITE);
        
    display.setCursor(0,p+=3);
    display.print(F("time: "));
    digitalClockDisplay();
    display.drawLine(0,p+=9,display.width(),p,BLACK);

    display.setCursor(0,p+=3);
    display.print(F("mode: "));
    display.println(str_mode);
    display.display();

    if(left() == true)
    {
      screen = SCREEN2;
    }
    
    if(right() == true)
    {
      screen = SCREEN3; 
    }

    if(enter() == true)
    {  
       if(mode == 3) {mode = 1;}
       else {mode++;}
       if(mode == 1) {str_mode = "MANUAL";}
       if(mode == 2) {str_mode = "OPTIMAL"; }
       if(mode == 3) {str_mode = "AUTO"; }   
    }    
}

void screen2()                                         //screen2 display speed and cadence
{
   display.clearDisplay(); 
       
   display.setTextSize(1);
   display.setCursor(0,0);
   display.println(F("speed:"));
   display.setTextSize(2);
   
   displaySpeed(wheelSpeed);
   display.setTextSize(1);
   display.println(F(" kph")); 

   display.drawLine(0,23,display.width(),23,BLACK);
      
   display.setTextSize(1);
   display.setCursor(0,25);
   display.println(F("pulse:"));
   display.setTextSize(2);
   
   displayRPM(rpmCrank);
   display.setTextSize(1);
   display.println(F(" rpm"));
      
   display.display();  

   if(left() == true)
   {
     screen = SCREEN3;
   }
    
   if(right() == true)
   {
     screen = SCREEN1; 
   }
}

void screen3()                                         //screen3 display distance, trip time, average speed
{
   int p = 2;
   display.clearDisplay();
   display.setTextSize(1);
    
   display.setCursor(0,p);
   display.print(F("STATS   "));
   display.print(str_stats);
   display.drawLine(0,p+=9,display.width(),p,BLACK);
      
   display.setCursor(0,p+=3);
   display.print(F("dist: "));

   if(distance < 10)
    display.print("0");
   display.print(distance,2);
   display.println(F(" km"));    
   display.drawLine(0,p+=9,display.width(),p,WHITE);
    
   display.setCursor(0,p+=3);
   display.print(F("time: "));
   displayTravelTime();
   display.drawLine(0,p+=9,display.width(),p,WHITE);
      
   display.setCursor(0,p+=3);
   display.print(F("avgs: "));
   Average();
   if(avg < 10)
    display.print("0");
   display.print(avg,1);
   display.println(F(" kph")); 
   
   display.display();

  if(left() == true)
  {
    screen = SCREEN1;
  }
    
  if(right() == true)
  {
    screen = SCREEN2; 
  }

  if(enter() == true)
  {
    if(long_press)
    {
      str_stats = "stop";
      long_press = false; 
      distance = 0; 
      avg = 0;
      mem_timer = 0;
      elap = 0;
    }
    
    if(str_stats == "start")
    {str_stats = "stop"; start_timer=true;}
    else
    {str_stats = "start"; start_timer=false;}
  }
}

void screen4()                                         //screen4 display start menu 
{
  while(true)
  {
    display.clearDisplay();
    display.setCursor(3,4);
    display.println(F("Enter menu?"));
    display.drawLine(0,14,display.width(),14,BLACK);
    
    display.setCursor(3,20);
    display.println(F("Y           N"));
    
    display.setCursor(3,30);
    display.println(F("    reset    "));
    
    display.setCursor(3,38);
    display.println(F("<     o     >"));
    
    display.display();
  
    if(left() == true)
    {
      screen5();
      Exit_Configuration_Menu=false;
    }
      
    if(right() == true)
    {
      SaveEEPROMGears();
      break;  
    }
  
    if(enter() == true)
    {
      EEPROM.write(E_INIT, 'N'); 
      break;
    }
  }
}

void screen5()                                         //screen5 display configuration menu 
{
  while(!Exit_Configuration_Menu)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(3,4);
    display.println(F("gear settings"));
    display.drawLine(0,14,display.width(),14,BLACK);
    
    display.setCursor(3,20);
    display.println(F("front    rear"));
    
    display.setCursor(3,30);
    display.println(F("     exit    "));
    
    display.setCursor(3,38);
    display.println(F("<     o     >"));
    
    display.display();
  
    if(left() == true)
    {
      screen6('F', F, Front, F_SIZE);
    }
      
    if(right() == true)
    {
      screen6('R', R, Rear, R_SIZE);
    }
  
    if(enter() == true)
    {
      Exit_Configuration_Menu = true;
    }
 }
}

void screen6(char desc,int Gear[],int const which_servo,int Size) //screen6 display calibration menu
{
  int Position = 0;
  
  for(int i=0; i<Size; i++)
  {
    if(i==0)
    {
      Position = Gear[0];
      servo[which_servo].write(Gear[0]);
    }
    else
    {
      if(Gear[i] != 0)
      {
       Position = Gear[i];
       servo[which_servo].write(Gear[i]);
      }
      else
      {
       servo[which_servo].write(Gear[i-1]);
      }
    }   

    if(which_servo == FrontServo)
    {
      gearF = i+1;
    }
    if(which_servo == RearServo)
    {
      gearR = i+1;
    }
    
    while(true)
    {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(3,4);
    display.println(F("gear position"));
    display.setTextSize(2);
    display.setCursor(3,14);
    display.print(desc);
    display.print(i+1);
    display.print(F(" "));
    display.print(Position);
    display.setTextSize(1);
    display.setCursor(3,30);
    display.println(F("     next    "));
  
    display.setCursor(3,38);
    display.println(F("<     o     >"));
  
    display.display();
    
      if(right() == true)
      {
        if(Position < MAX_RANGE)
        {
          Position = Position+5;
          servo[which_servo].write(Position);
          delay(150);
        }    
      }

      if(left() == true)
      {
        if(Position > MIN_RANGE)
        {
          Position = Position-5;
          servo[which_servo].write(Position);
          delay(150);
        }
      }

     if(enter() == true)
     {
         Gear[i] = Position; 
         break;  
     }
    }       
  }
} 
                                   
void ShiftDisplay(int f, int r )                       //function to display gear positions on the shifter screen
{
   int g = f * r;
   
   u8x8.setFont(u8x8_font_8x13B_1x2_r);
   u8x8.drawGlyph(2, 0, 'F');
   u8x8.drawGlyph(5, 0, 'R');
   
   itoa(f, sf, 10);
   u8x8.drawString(2, 2, sf);
   
   itoa(r, sr, 10);
   u8x8.drawString(5, 2, sr);
   
   u8x8.drawString(8, 0, "Shift");
   u8x8.drawString(10, 2, "  ");
   
   itoa(g, sg, 10);
   u8x8.drawString(10, 2, sg);

}

void ShiftMode(int m)                                  //function to display drive mode on the shifter screen
{
   if(m == 1)
   {
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawGlyph(14, 0, 'M');
    u8x8.drawGlyph(14, 1, 'A');
    u8x8.drawGlyph(14, 2, 'N');
    u8x8.drawGlyph(14,  3, ' ');
   }
   if(m == 2)
   {
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawGlyph(14, 0, 'O');
    u8x8.drawGlyph(14, 1, 'P');
    u8x8.drawGlyph(14, 2, 'T');
    u8x8.drawGlyph(14,  3, ' ');
   }
   if(m == 3)
   {
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawGlyph(14, 0, 'A');
    u8x8.drawGlyph(14, 1, 'U');
    u8x8.drawGlyph(14, 2, 'T');
    u8x8.drawGlyph(14,  3, 'O');
   }   
}

bool left()                                            //detection of pressing left button on main screen
{
  bool tmp = false;
  if (digitalRead(SwLeft)== LOW)
  {
    delay(20);
    tmp = true;
    while(digitalRead(SwLeft)== LOW) 
    delay(20);
  } 
  if(tmp){return true;}
  else{return false;}
}

bool right()                                           //detection of pressing right button on main screen
{
  bool tmp = false;
  if (digitalRead(SwRight)== LOW)
  {
    delay(20);
    tmp = true;
    while(digitalRead(SwRight)== LOW) 
    delay(20);
  }
  if(tmp){return true;}
  else{return false;}
}

bool enter()                                           //detection of pressing center button on main screen
{
  bool tmp = false;
  if (digitalRead(SwEnter)== LOW)
  {
    delay(20);
    tmp = true;
    start = millis();
    
    while(digitalRead(SwEnter)== LOW)
    {
      finish = millis();
      elapsed = (finish - start);
      if(elapsed > 2000 and elapsed < 2020)
      {
         display.setCursor(78,2);
         display.print("^");
         display.display();
         long_press = true;
      }
    } 
  }
  if(tmp){return true;}
  else{return false;}
}

bool isBtnPressed(int analogVal, int btnVal)           //function to check which shifter button is pressed 
{
  return abs(btnVal - analogVal) <= 30;
}

void Shifter(int adc)                                  //function to assign action on each shifter button
{
  if(isBtnPressed(adc, BTN1))
  {
    ShiftUp(Front);  
  }
  else if(isBtnPressed(adc, BTN2))
  {
    ShiftDown(Front);
  }
  else if(isBtnPressed(adc, BTN3))
  {
    ShiftUp(Rear); 
  }
  else if(isBtnPressed(adc, BTN4))
  {
    ShiftDown(Rear);
  }
}

void ShiftDown(const int which_servo)                  //function to shift gear up 
{
  int actual_position = 0 ;
  if(which_servo == FrontServo and gearF > 1)
  {
    gearF--;
    actual_position = F[gearF - 1]; 
    EEPROM.write(11,gearF);          
  }
  if(which_servo == RearServo and gearR > 1)
  {
    gearR--;
    actual_position = R[gearR - 1];
    EEPROM.write(12,gearR);         
  } 
  if(actual_position != 0)
  {
    //servo[which_servo].write(actual_position - 5);
    //delay(200);
    servo[which_servo].write(actual_position);
    delay(100);
  }
}

void ShiftUp(const int which_servo)                    //function to shift gear down
{
  int actual_position = 0;
  if(which_servo == FrontServo and gearF < F_SIZE)
  {
    gearF++;
    actual_position = F[gearF - 1];  
    EEPROM.write(11,gearF); 
  }
  if(which_servo == RearServo and gearR < R_SIZE)
  {
    gearR++; 
    actual_position = R[gearR - 1];
    EEPROM.write(12,gearR);   
  }
  if(actual_position != 0)
  {
   //servo[which_servo].write(actual_position + 5);
   //delay(200); 
   servo[which_servo].write(actual_position);
   delay(100);
  
  }
}

void CheckAnalogButton()                               //function check if shifter button is pressed
{
  int analogValue = analogRead(analogSw);
  if(analogValue > 250)
   {
    delay(20);
    Shifter(analogValue);
    ShiftDisplay(gearF, gearR);
    while(analogRead(analogSw) > 250) 
    delay(20);
  } 
}

void InitializeGears()                                 //first initialization of gears array 
{
  F[0]=10;
  R[0]=10;
  for(int i=1; i<F_SIZE; i++)
  {
    F[i] = 0;
  }
  for(int i=1; i<R_SIZE; i++)
  {
    R[i] = 0;
  }
}

void SetEEPROMGears()                                  //function to set saved gears position to gears array 
{
  for(int i=0; i<F_SIZE; i++)
  {
   F[i] = EEPROM.read(i);    //set gears to right position
  }
  for(int i=0; i<R_SIZE; i++)
  {
   R[i] = EEPROM.read(i+3);  
  }
  
  gearF = EEPROM.read(11);  //read the last position
  gearR = EEPROM.read(12);  
}

void SaveEEPROMGears()                                 //function to save new gear positions in EEPROM 
{
  for(int i=0; i<F_SIZE; i++)
  {
    EEPROM.update(i,F[i]);
  }
  for(int i=0; i<R_SIZE; i++)
  {
    EEPROM.update(i+3,R[i]);
  } 
}

void ServoInit()                                       //initialization of servos and assign current positions
{
  servo[FrontServo].attach(4);
  servo[FrontServo].write(F[gearF-1]); 
 
  servo[RearServo].attach(8);
  servo[RearServo].write(R[gearR-1]);
}

void setup()                                           //function to initializate system 
{
  str_mode = "MANUAL";
  str_stats = "start";
  /*initialization OLED and LCD*/
  u8x8.begin();   
  display.begin(65,0x04);                  
  display.clearDisplay();                  
  display.setTextColor(BLACK); 
  
  /*initialization interupt handler*/
  attachInterrupt(1, crankCounter, RISING); 
  attachInterrupt(0, wheelCounter, RISING);
  
  /*sync RTC*/
  setSyncProvider(RTC.get);
  
  /*initialization inputs*/
  pinMode(SwLeft,INPUT_PULLUP);
  pinMode(SwRight,INPUT_PULLUP);
  pinMode(SwEnter,INPUT_PULLUP);
  pinMode(analogSw, INPUT_PULLUP);
  
  /*first initialization of gears array*/
  InitializeGears();

  /*If initial configuration was made after
  first system boot then assign memory positions
  to gears array, set servos and current 
  position, display initial menu.*/
  if (EEPROM.read(E_INIT) == 'T')
  {
    SetEEPROMGears();
    ServoInit();
    screen4();
  }
  /*Else set servos, execute initial 
  configuration, save position in memory
  and set flag which describe the factory 
  settings was changed.
  */
  else
  {
    ServoInit();
    screen6('F', F, Front, F_SIZE);
    screen6('R', R, Rear, R_SIZE);
    SaveEEPROMGears();
    EEPROM.write(E_INIT, 'T');
  }
  /*initial configuration of shifter screen*/
  ShiftDisplay(gearF, gearR);
}

void loop()                                            //main loop
{
  /*based on variable choose screen from 1 to 3*/
  switch(screen)  
  {
    case SCREEN1:
        screen1();
        break;
        
    case SCREEN2:
        screen2();
        break;
        
    case SCREEN3:
        screen3();
        break; 
  }
  
  CheckAnalogButton();   /*check if shifter button was pressed*/                         
  ShiftMode(mode);       /*update drive mode*/
  RPM();                 /*cadence measurement*/
  SPEED();               /*speed measurement*/
}