#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#define Key 8                                //pin 8 to change mode from AT to communication
#define Vcc 9                                //pin 9 turn on transistor and power BT module
#define D1 2                                 //led indicating state (communication or wait for communication)
#define sw1 3                                //switch to exit AT mode

//variables to init ethernet web serwer 
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

SoftwareSerial BT(6, 7);                     //(TX,RX) Serial communication with bluetooth Module

char slave;
char command;                                //char variable to store command for mortor
char buf[7];                                 //char array to recive data from slaves
String addr;                                 //address of each slave 
int input_value = 0;                         //new post value from user 
int V1, V2;                                  //values from slave's light sensor 
int avr_value = 0;                           //value redy to send
int bytes;                                   //checks how many bytes was recived from BT module
boolean stopinit = true, change = true;      //when false, AT mode in setup ends
unsigned long start = 0, finished, elapsed;  //to mesaure exe time
char ChAddr[]={"AT+BIND=xxxx,xx,xxxxxx\r\n"};//char address of BT module

//variables in web communication
String POST_req;                             //save the HTTP POST request
String lastvalue ="";                        //last input value in web page
char post[6];                                //array to keep extract value from http post reguest

void setup() {
  //communication serial and ethernet setup
  Serial.begin(9600);
  BT.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(sw1, INPUT_PULLUP);
  pinMode(D1, OUTPUT);
  pinMode(Key, OUTPUT);
  pinMode(Vcc, OUTPUT);
  //-----------------------------------------
  // Enter AT command baud 9600
  //Serial.println("inicalizacja trybu AT:");
  digitalWrite(Key, LOW);
  delay(1000);
  digitalWrite(Vcc, HIGH);
  delay(1000);
  // -----------------------------------------
  // init mode
  //Serial.println("AT+BIND=FCA8,9A,005CD6");
  digitalWrite(Key, HIGH);
  delay(250);
  BT.write("AT+BIND=FCA8,9A,005CD6\r\n");    //first connect to slaveA 
  delay(250);
  while(BT.available())                      //clear serial buffer from data
  {
    Serial.write(BT.read());
  }
  //Serial.println("wciśnij przycisk aby rozpocząć transmisję ");
  while (stopinit) {
    if (digitalRead(sw1) == LOW)             //if sw1 pressed exit AT command end enter to loop
    {
      delay(20);
      stopinit = false;
      digitalWrite(Key, LOW);
    } 
    if (Serial.available())                  //read and write data from BT module to serial port
    {
      BT.write(Serial.read());
    }
    if (BT.available())
      Serial.write(BT.read());                                    
  }
}

void loop() {

  while(!BT.available())                     //send 'r' request until get answer from BT module
  {
    BT.write('r');
    digitalWrite(D1, HIGH);
    delay(100);
    SendToWeb();                             //all communication with server and send data to web
  }
  
  digitalWrite(D1, LOW);
  char answer = BT.read();                   //read 1 byte command from BT serial and decide what to do
  slave = answer;
  Serial.print("Odebrano znak: ");
  Serial.println(answer);
  Serial.print("połączono z: ");

  switch(answer){                            // in case of specific command 
    case 'a':
    Serial.println("Czujnik A");
    SaveData(slave);
    Switch_Slave(slave);                              
    break;
    case 'b':
    Serial.println("Czujnik B");
    SaveData(slave);
    Switch_Slave(slave);                           
    break;
    case 'c':
    Serial.println("Sterownik");
    Regulation();
    BT.write(command);
    Switch_Slave(slave);
    break;
  }
  Serial.print("czas przełączenia: ");
  CheckTime();
  Serial.println("s");
  Serial.println("");
}

void SaveData(char sign){

  bytes = BT.readBytesUntil('e',buf,7);
  Serial.print("ramka danych: ");
  Serial.print(sign);
  Serial.print(buf);
  Serial.println("e");
  Serial.print("wartość pomiaru: ");
  Serial.println(buf);
  switch(sign){                              // in case of specific command 
    case 'a':
    V1 = atoi(&buf[0]);
    avr_value = (V1 + V2)/2;
    break;
    case 'b':
    V2 = atoi(&buf[0]);
    avr_value = (V1 + V2)/2;
    break;
  }
  Serial.print("wartość średnia: ");
  Serial.print(V1);
  Serial.print("+");
  Serial.print(V2);
  Serial.print("=");
  Serial.println(avr_value);                
}

void Switch_Slave(char sign){

  Serial.print("zmiana adresu: ");
  BT.write('e');
  switch(sign){                                // in case of characteristic slave sign set the next address
    case 'a':  
    addr = "AT+BIND=0018,E4,400006\r\n";
    break;
    case 'b':
    addr = "AT+BIND=98D3,32,F5ABBA\r\n";
    break;
    case 'c':
    addr = "AT+BIND=FCA8,9A,005CD6\r\n";
    break;
  }
  addr.toCharArray(ChAddr,sizeof(ChAddr));
  digitalWrite(Key,HIGH);
  delay(200);
  BT.write(ChAddr);
  delay(200);
  while(BT.available())                       //clear serial buffer from data
  {
    Serial.write(BT.read());
  }                                  
  digitalWrite(Vcc,LOW);                      //restart master module
  digitalWrite(Key,LOW); 
  delay(500);
  digitalWrite(Vcc,HIGH);
  addr = "";
  memset(buf, '\0', 7);                       //clear buffer data 
}

void Regulation(){
 
   if (input_value > 0)
   {
     if((avr_value < input_value + 50) and (avr_value > input_value - 50))
     {
       command = 's';
     }
     else if(avr_value < input_value - 50){
       command = 'u';
     }else{
       command = 'd';
     } 
   }else{command = 's';} 

   Serial.print("wysłano komendę: ");
   Serial.println(command);
}

void SendToWeb(){

    EthernetClient client = server.available(); // listen for incoming clients

    if (client)
    {  
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {   
                char c = client.read(); 
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    while(client.available())
                    {
                      char c1 = client.read();
                      POST_req += c1;
                    }
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connnection: close");
                    client.println();
                    //web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Light Control Page</title>");
                    client.println("<meta http-equiv=\"refresh\" content=\"1\">");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h2>Control Center</h2>");
                    client.println("<br>");
                    client.println("<form method=\"post\">");
                    client.println("Set the light level ");
                    client.println("<input type=\"text\" name=\"textbox\" maxlength=\"5\" style=\"width: 60px\" >");
                    client.println("<input type=\"submit\" value=\"send\" style=\"width: 60px\">");
                    client.println("in lux");
                    ProcessPost(client);
                    client.println("<br><br>");
                    client.println("</form>");
                    CurrentValue(client);
                    AdjustedValue(client);
                    client.println("<script>");
                    client.println("if ( window.history.replaceState ) {");
                    client.println("window.history.replaceState( null, null, window.location.href );");
                    client.println("}");
                    client.println("</script>");
                    client.println("</body>");
                    client.println("</html>");
                    break;
                }
                if (c == '\n') {
                    currentLineIsBlank = true;   // you're starting a new line         
                } 
                else if (c != '\r') {
                    currentLineIsBlank = false;  // you've gotten a character on the current line
                }
            } 
        } 
        delay(1);                                // give the web browser time to receive the data      
        client.stop();                           // close the connection:
        POST_req = "";                           // clear post reguest
        memset(post, '\0', 6);                   
    }  
}

void ProcessPost(EthernetClient cl){
  
    if (POST_req.indexOf("textbox=") > -1)
    {
        for(int i=0; i<5; i++)                   //max 5 digits number
        {
          if(isWhitespace(POST_req.charAt(i+8))) //from 8th sign read value, when space end of value 
          {
            break;
          }
          post[i] = POST_req.charAt(i+8);        //write value from post reguest to post array
        }   
    }       
}  

void CurrentValue(EthernetClient c2){
  //function upating the current value of light sensor argage from slaveA and SlaveB 
  //put to the web page
  String value = String(avr_value);
  c2.println("<p>Current value: <input type=\"text\" name=\"Cv\" value=\"" + 
  value + "\" style=\"width: 60px\" readonly>" );
}

void AdjustedValue(EthernetClient c3){
  //function update adjusted value and put it to the web page
  if(post[0] != '\0')
  {  
    String value = String(post); //parse char array (post) to String
    //Serial.println("===========================");
    //Serial.print("nowy poziom nasłonecznienia: ");
    //Serial.println(value);
    //Serial.println("===========================");
    input_value = value.toInt();
    lastvalue = value;
    c3.println("<p>Adjusted value: <input type=\"text\" name=\"Av\" value=\"" + 
    value + "\" style=\"width: 60px\" readonly>" );
  }else{
    c3.println("<p>Adjusted value: <input type=\"text\" name=\"Av\" value=\"" + 
    lastvalue + "\" style=\"width: 60px\" readonly>" );
  }
}

void CheckTime(){
  finished = millis();
  elapsed = (finished - start)/1000;
  start = millis(); 
  Serial.print(elapsed); 
}