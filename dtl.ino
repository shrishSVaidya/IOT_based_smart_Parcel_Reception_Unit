#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

Servo myservo1;
Servo myservo2;
Servo myservo3; 

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

#define servo1pin  D1
bool servo1status = HIGH;

#define servo2pin  D2
bool servo2status = HIGH;

#define servo3pin D3
bool servo3status= HIGH;

unsigned int bookings[3]={0,0,0};
unsigned short int verifies[3]={0,0,0};
short int flag=-1, got=0, take=0, verified=0, servo=-1, closing=0, start=0;

void setup() {
  Serial.begin(115200);
  
  myservo1.attach(servo1pin);
  myservo2.attach(servo2pin);
  myservo3.attach(servo3pin);

  WiFi.softAP(ssid, password);
//  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  IPAddress ip= WiFi.softAPIP();
  Serial.println(ip);
  server.on("/", handle_OnConnect);
  server.on("/book", booking_process);
  server.on("/open", take_password);
  server.on("/got", got_it);
  server.on("/close", close_door);
  server.on("/action_page", handleform_n_verify);
  server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println("HTTP server started");

  myservo1.write(0);
 
  myservo2.write(0);
  
  myservo3.write(0);

}
void loop() {
  server.handleClient();
}

void handleform_n_verify()
{
  int i=0;
  start=1;
  String password= server.arg("password");
  Serial.print("Password recieved: ");
  Serial.println(password);
  take=0;

  // To verify:
  for (i=0; i<3; i++)
  {
    if (String(bookings[i])==password)
    {
      verified=1;
      verifies[i]+=1;
      closing=0;
      servo=i;
      if (verifies[i]==2)
      {
        verifies[i]=0;
        bookings[i]=0;
      }
      break;
    }
  }
  
  if (i==3)
  {
    servo=-1;
    verified=0;
    server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
  }
  else
  open_door();

}

void open_door()
{
  //turn respective servo by 150
  Serial.print("open door: ");
  Serial.println(servo+1);
  if (servo==0)
  myservo1.write(150);

  if (servo==1)
  myservo2.write(150);

  if (servo==2)
  myservo3.write(150);

    server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
}

void close_door()
{
  // rotate the respective servo to 0
  Serial.print("Closing door: ");
  Serial.println(servo+1);

  if (servo==0)
  myservo1.write(0);

  if (servo==1)
  myservo2.write(0);

  if (servo==2)
  myservo3.write(0);

  closing=1;
  
  server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
}

void got_it()
{
  got=1;
  server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
}

void take_password()
{
  //take password through html forms
  take=1;
  server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
}



void handle_OnConnect() {
  servo1status = LOW;
  servo2status = LOW;
  servo3status = LOW;
  Serial.println("servo1 Status: CLOSE | servo2 Status: CLOSE | servo3 status: CLOSE");
  server.send(200, "text/html", SendHTML(servo1status,servo2status, servo3status));
}


//void handle_servo3off() {
//  servo3status = LOW;
//  Serial.println("servo3 Status: CLOSE");
//  server.send(200, "text/html", SendHTML(servo1status,servo2status, false));
//}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void booking_process(){
  int i=0;
  for (i=0; i<3; i++)
  {
    if (bookings[i]==0)
    {
      //random password generator for that slot
      bookings[i] = random(999, 10000);
      flag=i;
      Serial.print("Generated password for slot no.");
      Serial.print(i+1);
      Serial.print(" is: "); 
      Serial.println(bookings[i]);
      got=0;
      break;
    }
  }
  if (i==3){
    //no bookings
    flag= -1;
    got=0;
  }

server.send(200, "text/html", SendHTML(servo1status,servo2status, false));
}

String SendHTML(uint8_t servo1stat,uint8_t servo2stat, uint8_t servo3stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #000000;margin: 50px auto 30px;} h3 {color: #000000;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: blue;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 20px;color: #000000;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>WELCOME TO DELIVEREASE.COM</h1>\n";
  ptr +="<h3>An attempt to revolutionalize the delivery system</h3>\n";
  ptr += "<p>Please select either of the buttons below to Book/Place/Access a parcel</p>\n";
  ptr +="<a class=\"button button-off\" href=\"/book\">Book Slot</a>\n";
  ptr +="<a class=\"button button-on\" href=\"/open\">OPEN</a>\n";

  if (flag!=-1 && got!=1)
  {
    ptr+="<p>You have booked slot no: ";
    ptr+=String(flag+1);
    ptr+="</p>\n";
    
    ptr+= "<p>Your password is: ";
    ptr+=String(bookings[flag]);
    ptr+="</p>\n";
    ptr+="<a class=\"button button-off\" href=\"/got\">Got it</a>\n";
  }
  if (flag==-1 && got!=1 )
  {
    ptr+="<p>Sorry, no Slots available</p>\n";
    ptr+="<a class=\"button button-off\" href=\"/got\">Got it</a>\n";
  }

  if (take==1)
  {
    ptr+="<form action=\"/action_page\">\n";
    ptr+= "<input type=\"text\" name=\"password\" autocomplete=\"off\" placeholder=\"Password ?\">\n";
    ptr+= "<input type=\"submit\" value=\"Confirm\"></form> \n";
  }

  if (verified==1 && closing !=1)
  {
    ptr+= "<p>Password verified</p>\n";
    ptr+= "<p>Slot number ";
    ptr+= String(servo+1);
    ptr+= " is open</p>\n";
    ptr +="<a class=\"button button-off\" href=\"/close\">Close</a>\n";
  }

  else if (verified==0 && start==1)
  {
    ptr+= "<p>Wrong Password</p>\n";
    
  }

  
  
//   if(servo1stat)
//  {ptr +="<p>servo1 Status: OPEN</p><a class=\"button button-off\" href=\"/servo1off\">CLOSE</a>\n";}
//  else
//  {ptr +="<p>servo1 Status: CLOSE</p><a class=\"button button-on\" href=\"/servo1on\">OPEN</a>\n";}


  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
