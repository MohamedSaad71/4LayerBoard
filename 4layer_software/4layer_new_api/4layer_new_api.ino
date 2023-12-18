#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <avr/wdt.h>
#include "SIM800L.h"
#include "DHT.h"
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <MG811.h>
#include <math.h>
/**************** PinOut of differrent modules ***************************/
#define big_battery_pin 63 //pin to read big battery value
#define small_battery_pin 65 //pin to read small battery value
#define relay_pin 26 //relay pin to switch between big and small batery and charging
#define buzzer 57
#define DHTPIN 38     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22 type
#define SIM800_RST_PIN 24 //reset pin for GSM
#define wind 60
#define en_fan 13
#define fan1 35
#define fan2 34
#define en_valve 46
#define valve1 45
#define valve2 44
#define mega_send 60
/***************** objects and classes ***********************/
DynamicJsonBuffer jsonBuffer; //create json object
DHT dht(DHTPIN, DHTTYPE); //create a dht11 object
RTC_DS1307 rtc; // Create a RealTimeClock object
DateTime  now ; // Read data from the RTC Chip
TinyGPSPlus gps; // create a gps object
File myfile; //create a sd card object
SIM800L* sim800l; //create instance of SIM800L class
MG811 mySensor = MG811(A8); // Analog input A14 for CO2
/**************************variables*************************/
float lati, loni;//latitude and longitude for gps
char lat[20], lon[20];//char array to make ready for send to http
String la, lo = "";//string for collect data of gps
int flag = 0; //to insure gprs doesn't send twice in same minute
int flag2 =0;// to prevent stm stuck if power off
int flag3=0; //to prevent recieving data multiple times from stm every three minutes
char battery_flag=14; //to check if small battery needs charging
char send_flag = 16;
int stmflag=0; //to request data only once from stm
int big_battery_percent;
int small_battery_percent;
unsigned int valve_new =0;
unsigned int valve_oldvalue = 0;
int x =0;//counter to check gsm 
int old_small, new_small ;
int old_meduim, new_meduim ;
int old_large, new_large ;
int old_fly, new_fly ;
int chipSelect = 53; //Chip select for the memory card
String FileName_SD_card="";//string to save th name of file in sd card
char filename[30]; //array of characters in which the string value of FilName_SD_card in saved fo operations
float v400 = 4.535; //variable for co2 sensor
float v40000 = 3.206; //variable for co2 sensor

const char Serialnumber[]= "\"eg-123-2020-098165\"";

// Define the input and output ranges
const int INPUT_START = 1;
const int INPUT_END = 100;
const int OUTPUT_START = 10;
const int OUTPUT_END = 64000;
void setup() {
  pinMode(big_battery_pin,INPUT);
  pinMode(small_battery_pin,INPUT);
  pinMode(relay_pin,OUTPUT);
  pinMode(mega_send,OUTPUT);
  pinMode(buzzer , OUTPUT);
  TCCR3B = TCCR3B & B11111000 | B00000100;   // for PWM frequency of 122.55 Hz
  Serial.begin(9600);//for monitoring
  while(!Serial);
  fan_init();
  delay(500);
//fan_start(200);
  valve_init();
  unsigned int mappedValue = map(25, 0, 100, 0, 64000);
  Serial.println( mappedValue);
  Serial1.begin(9600);//for gsm
  while(!Serial1);
  Serial2.begin(9600);//for counter
  Serial3.begin(9600);//for gps
  delay(1000);
  mySensor.begin(v400, v40000);
  GPS_run();
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  rtc.begin();

  //RTC_init();
  
  DHT11_init();
  DHT11_run();
  InitializeSDcard();
  createFile("newfile.txt");
  Serial.println(mySensor.read());
   if(analogRead(big_battery_pin)>370)  //check big battery reading and if connected
 {
  digitalWrite(relay_pin,HIGH);
  EEPROM.write(battery_flag,1);
 // valve_start(mappedValue);
 }
  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  sim800l = new SIM800L((Stream *)&Serial1, SIM800_RST_PIN, 200, 512);
  sim800l->reset();
  setupModule();
  connect_check();
}

void loop() {
 DateTime now = rtc.now();
 Serial.println(" ");

 if(analogRead(big_battery_pin)>370)  //check big battery reading and if connected
 {
  Serial.print("Big Battery:");
  Serial.println(analogRead(big_battery_pin));
  if( (analogRead(small_battery_pin)== 0)  ||  (analogRead(small_battery_pin)<760 ) ){
       EEPROM.write(battery_flag,1);
      }
  if(  analogRead(small_battery_pin)>=1020 ){
      Serial.println("Not charging");
      digitalWrite(relay_pin,LOW);
      EEPROM.write(battery_flag,0); 
      }
  else if(( EEPROM.read(battery_flag) && analogRead(small_battery_pin)<1020)) {
      //Serial.println("charging");
      digitalWrite(relay_pin,HIGH);
    }
  else{}
   Serial.print("Small Battery:");
   Serial.println(analogRead(small_battery_pin));
   //Serial.print("Battery flag:");
   //Serial.println(EEPROM.read(battery_flag));
  /*Receiving data from STM32 every 3 mins and save the readings*/
  if(now.minute()%3==0&&flag3==0)
  {
    Serial.println("Receiving data from 328");
    RTC_run();
    ReceiveData_counter();
    flag3=1;
    delay(100);
    check_recieve();
  }
  else if(!(now.minute()%3==0))
  {
   // Serial.println("not time for 328");
    flag3=0;
    stmflag=0;
    delay(1000);
  }
  else{}
  if(((now.minute()%5) == 0) && (EEPROM.read(send_flag)==0) )
  {
      RTC_run();
      DHT11_run();
      send_data();
      EEPROM.write(send_flag,1);              
   }                      
  else
    {
      if(!((now.minute()%5 )==0))
      {

        //Serial.println("not time for atmega");
        RTC_run();
        EEPROM.write(send_flag,0);
      }
      else{}
    }
}

 
else
{
  digitalWrite(relay_pin,LOW);
  Serial.print("Small Battery:");
  Serial.println(analogRead(small_battery_pin));

  if( ((now.minute()%2) == 0) && (flag ==0) )
  {
     Serial.println("small Battery");
     Serial2.write('E');  //send command to enter emergency
     emergency_send();
     flag =1;
     myfile = SD.open("newfile.txt", FILE_WRITE);       
     myfile.println(analogRead(small_battery_pin));
     myfile.close();
     delay(12000);
              /*
              * These four lines for Displaying from SD card the Readings for small Battery
              *
              */
//   Serial.println("Reading the data from the Memory Card");
//   myfile = SD.open("newfile.txt", FILE_READ);
//   readFile();
//   closeFile();
     Serial.print("resetting");
     wdt_enable(WDTO_8S);
     delay(9000);
  }
  else if( !((now.minute()%2 )==0) )
  {
     //Serial.println("not time yet");
     RTC_run();
     flag=0;
  }  
 }
 
}
