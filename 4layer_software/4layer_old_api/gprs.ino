char APN[30] = " TM"; //char APN[30] = "internet.vodafone.net" ;//Internet.be
char check[100];//used to recieve data from website
char data[400]; //used to collect and send data 
char emergency[100];//used to collect emergency data
uint16_t rc = 0;//to document http reply
const char CONTENT_TYPE[] = "application/json";
const char URL[] = "http://appno.epcmms.com/api/Traps/LoadData2";//api for recieving data 
const char URL2[] = "http://appno.epcmms.com/api/Reading";//api to send data
const char URL3[] = "http://appno.epcmms.com/api/TrapEmergencies/AddTrapEmergency";
bool connected = false;
String D, T;//string for collect date and time
char Date[11], Time[6];//char array to make ready for send to http

String sdrequest ;
char Minutes[8]; /***********This array for just saving the current minutes to the sd card             *********/

int i=0, j=0; // for saving the data from the counter to array
String counter[6];// Holding the counters
String c="";//used to hold data from counter

char small[8];
char meduim[8];
char large[8];
char fly[8];

//flags to create new file in case of date change
int Day_flag=0;
int Month_flag=0;
int Year_flag=0;
int time_flag=1;
/************* For the SD card  ***************/
String Data_name(String u)
{
  String Name = "";
  boolean c = false;
  for (int o = 2; o <= u.length(); o++)
  {
    if (u.charAt(o) != '-' )
    {
      Name += u.charAt(o);
    }
    else if (u.charAt(o) == '-' && c == false)
    {
      c = true;
      Name += u.charAt(o);
    }
    else if (u.charAt(o) == '-' && c == true)
    {
      Name += u.charAt(o);
      Name += u.charAt(o + 1);
      Name += u.charAt(o + 2);
      Serial.print("Name=");
      Serial.println(Name);
      return Name;
      break;
    }
    else{}
    wdt_reset();//reset WDT
  }
}

void receiveEvent(int howMany)
{
  flag2++;
  if(stmflag==0)
  {
    while (0 < Wire.available())
     {
       //Serial.println("Inside the function");
       c = Wire.readStringUntil('}');      /* receive byte as a character */
      }
    Serial.println(c);
    for (int v = 0; v <= c.length(); v++)
     {
       if (c.charAt(v) == ',')
         {
           counter[i] = c.substring(j, v);
           j = (v + 1);
           i++;
          }
       }
    c = "";
    j = 0;
    i = 0;
    stmflag=1;
    }
  else{}
}

//save stm data to eeprom
void ReceiveData_counter()
{
   digitalWrite(mega_send,HIGH);
   delay(2000);
   digitalWrite(mega_send,LOW);
   Serial.print("Data Received: ");
// Serial.println(counter);
   old_small = EEPROM.read(3);
   new_small = counter[0].toInt() + old_small;
   EEPROM.write(3, new_small);
  
   old_meduim = EEPROM.read(4);
   new_meduim = counter[1].toInt() + old_meduim; 
   EEPROM.write(4, new_meduim);

   old_large = EEPROM.read(5);
   new_large = counter[2].toInt() + old_large;
   EEPROM.write(5, new_large);

   old_fly = EEPROM.read(7);
   new_fly = counter[3].toInt() + old_fly;
   EEPROM.write(7, new_fly);

   String small_read_ee = String(EEPROM.read(3));
   String meduim_read_ee = String(EEPROM.read(4));
   String large_read_ee = String(EEPROM.read(5));
   String fly_read_ee = String(EEPROM.read(7));

   Serial.print("small=");
   Serial.println(EEPROM.read(3));
   Serial.print("meduim=");
   Serial.println(EEPROM.read(4));
   Serial.print("large=");
   Serial.println(EEPROM.read(5));
   Serial.print("fly=");
   Serial.println(EEPROM.read(7));
   Serial.print("counter problem");
   Serial.println(counter[4]);
   Serial.print("wind");
   Serial.println(counter[5]);
   if (counter[4] == "0"){
      counter[4] = "true";
      }
   else if (counter[4] == "1"){
      counter[4] = "false";
      }
   else{
      counter[4]="true";
      }
   small_read_ee.toCharArray(small, small_read_ee.length() + 1);
   meduim_read_ee.toCharArray(meduim, meduim_read_ee.length() + 1);
   large_read_ee.toCharArray(large, large_read_ee.length() + 1);
   fly_read_ee.toCharArray(fly, fly_read_ee.length() + 1);

   small_read_ee = "";
   meduim_read_ee = "";
   large_read_ee = "";
   fly_read_ee = "";
}
//setup sim800 module
void setupModule()
{
    // Wait until the module is ready to accept AT commands for 10 sec
  while(!sim800l->isReady() && x<15 )
   {
      Serial.println(F("Problem to initialize AT command, retry in 1 sec"));
      delay(1000);
      x++;
    }
  Serial.println(F("Setup Complete!"));
  x=0;
  // Wait for the GSM signal for 10sec
  uint8_t signal = sim800l->getSignal();
  while(signal <= 1 && x<15) 
   {
      delay(1000);
      signal = sim800l->getSignal();
      x++;
    }
  Serial.print(F("Signal OK (strenght: "));
  Serial.print(signal);
  Serial.println(F(")"));
  delay(1000);
  x=0;
  // Wait for operator network registration (national or roaming network)for 20 sec
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while(network != REGISTERED_HOME && network != REGISTERED_ROAMING && x<15)
   {
     delay(1000);
     network = sim800l->getRegistrationStatus();
     x++;
    }
  Serial.println(F("Network registration OK"));
  delay(1000);
  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(APN);
  for (int w = 0; w <= 10; w++) 
   {
     if  (!success) 
     {
       success = sim800l->setupGPRS(APN);
       delay(1000);
      }
    }
  Serial.println(F("GPRS config OK"));
}

void check_recieve(){
  //digitalWrite(sleep_pin,LOW);
  wdt_disable();
  Serial.println(F("Start HTTP POST..."));

  // Do HTTP POST communication with 20s for the timeout (read)
  sprintf(check, "{\"Serial\":%s}",Serialnumber);
  rc = sim800l->doPost(URL, CONTENT_TYPE, check, 20000, 20000);
  if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received Serial : "));
    Serial.println(sim800l->getDataReceived());
    JsonObject& doc = jsonBuffer.parseObject(sim800l->getDataReceived());
    /////////////////check the status update/////////////////////////

//    String val1 = doc["status"];
    String val2 = doc["co2"];
    String val3 = doc["fan"];
//    boolean val4 = doc["read"];
//    String val5 = doc["file"];
    sdrequest = (const char*) doc["fileDate"];
    Serial.println(sdrequest);
    jsonBuffer.clear();
  //valve conditions
    if (val2 == "0") {
        valve_new = 0;
        valve_start(valve_new);
       }
    else if (val2 == "25") {
       valve_new = map(25, 0, 100, 0, 64000);
       valve_start(valve_new);
      }
    else if (val2 == "50") {
       valve_new =map(50, 0, 100, 0, 64000);
       valve_start(valve_new);
      }
    else if (val2 == "75") {
       valve_new = map(75, 0, 100, 0, 64000);
     //  valve_start(valve_new);
      }
    else if (val2 == "100") {
       valve_new = map(100, 0, 100, 0, 64000);
       valve_start(valve_new);
      }
    else{}
  // fan conditions
    if (val3 == "0")fan_start(0);
    else if (val3 == "25")fan_start(125);
    else if (val3 == "50") fan_start(150);
    else if (val3 == "75")fan_start(175);
    else if (val3 == "100")fan_start(200);
    else{}
  } 
  else {
    // Failed...
     Serial.print(F("HTTP POST error "));
     Serial.println(rc);
    }
  wdt_reset();
}

void send_data(){

  int temperature = dht.readTemperature(); // Read temperature value
  int humidity = dht.readHumidity(); // Read humidity value

  now = rtc.now();  // Read data from the RTC Chip
  D =  String(now.year(), DEC) + "-" + String (now.month(), DEC) + "-" + String (now.day(), DEC);
  T = String(now.hour(), DEC) + ":" + String( now.minute(), DEC);
  D.toCharArray(Date, 11);
  T.toCharArray(Time, 6);
  Serial.println(Date);
  Serial.println(Time);
  GPS_run();
  la.toCharArray(lat, 10);
  lo.toCharArray(lon, 10);
  Serial.println("location array char:"); 
  Serial.println(lat);
  Serial.println(lon);
  delay(1000);
  /////////////////////////////CO2//////////////////////////////////////////
  int CO2_value = mySensor.read();
  Serial.print("C02 Concetration: ");
  Serial.print(CO2_value);
  Serial.print(" ppm");
   big_battery_percent = map(analogRead(big_battery_pin),0,980,0,100);
  small_battery_percent = map(analogRead(small_battery_pin),0,1010,0,100);

  if(small_battery_percent <0)
  {
    small_battery_percent=0;
  }
  else if(small_battery_percent >100)
  {
    small_battery_percent =100;
  }
  if(big_battery_percent > 100)
  {
    big_battery_percent = 100;
  }
 //  int readingtime = (now.hour()*4) + (now.minute()/15);   /**********uncomment****************/
  sprintf(data, "{\"counter\":\"1\",\"readingsmall\":\"%s\",\"readingMosuqitoes\":\"%s\",\"readingLarg\":\"%s\",\"readingFly\":\"%s\",\"BigBattery\":\"%d\",\"SmallBattery\":\"%d\",\"readingTempIn\":\"%d\",\"serlNum\":%s,\"readingTempOut\":\"%d\",\"readingHumidty\":\"%d\",\"readingDate\":\"%s\",\"readingTime\":\"%s\",\"readingLat\":\"%s\",\"readingLng\":\"%s\",\"readingWindSpeed\":\"%d\",\"co2\":\"%d\",\"co2Val\":\"%d\",\"isDone\":\"true\",\"isClean\":\"%s\"}"
          , small, meduim, large , fly, big_battery_percent, small_battery_percent ,temperature, Serialnumber,temperature, humidity , Date, Time, lat, lon, 0, CO2_value,counter[4]);
  
  /*******************uncomment also********************/
  /*
  sprintf(data, "{\"counter\":\"1\",\"readingsmall\":\"%s\",\"readingMosuqitoes\":\"%s\",\"readingLarg\":\"%s\",\"readingFly\":\"%s\",\"BigBattery\":\"%d\",\"SmallBattery\":\"%d\",\"readingTempIn\":\"%d\",\"serlNum\":%s,\"readingTempOut\":\"%d\",\"readingHumidty\":\"%d\",\"readingDate\":\"%s\",\"readingTime\":\"%s\",\"readingLat\":\"%s\",\"readingLng\":\"%s\",\"readingWindSpeed\":\"%d\",\"co2\":\"%d\",\"co2Val\":\"%d\",\"isDone\":\"true\",\"isClean\":\"%s\",\"ReadingNumber\":\"%d\"}"
          , small, meduim, large , fly, big_battery_percent, small_battery_percent ,temperature, Serialnumber,temperature, humidity , Date, Time, lat, lon, 0, CO2_value,counter[4],readingtime);
  */
  
  Serial.println(data);
  Serial2.write('r');  //send command to reset counter
  ////////////////////////////////////////////////////////////////////////
  
  /***************** Creating a new file each day to save the readings                                **********************/
   
  if(now.minute()!=EEPROM.read(9))
   {
    EEPROM.write(9,now.day());
    Day_flag=1;
   }
  else if(now.month()!=EEPROM.read(10))
   {
    EEPROM.write(10,now.month());
    Month_flag=1;
   }
  else if(now.year()!=EEPROM.read(11))
   {
    EEPROM.write(11,now.year());
    Year_flag=1;
   }
  else
   {
    Day_flag=0;
    Month_flag=0;
    Year_flag=0;  
   }
   
   Serial.println(EEPROM.read(9));
  if(Day_flag==1||Month_flag==1||Year_flag==1)
   {
    FileName_SD_card = Data_name(D);
    FileName_SD_card.toCharArray(filename, FileName_SD_card.length() + 1);
    FileName_SD_card = "";
    sprintf(filename, "%s.txt", filename);
    createFile(filename);
   }
      

  Serial.println(filename);

  /******************** Saving the data in the memory card                      ***********************/
   
        
  if(SD.exists(filename)){
           myfile = SD.open(filename, FILE_WRITE);
           myfile.println(data);
           myfile.close();
        }
  else{
           createFile(filename);
           myfile = SD.open(filename, FILE_WRITE);
           myfile.println(data);
           myfile.close();
        }
  

  Serial.println("Data stored");
  // Check to see if the file exists:
  if (SD.exists(filename)) 
  {
    Serial.println("File exists.");
  }
  else
  {
    Serial.println("File doesn't exist.");
  }

    /*********************************************************************/
    /************Reading The Data Saved in SD Card************************/
//  Serial.println("Reading the data from the Memory Card");
//  myfile = SD.open(filename, FILE_READ);
//  readFile();
//  closeFile();
 
  delay(1000);

  DateTime now = rtc.now();
  

  Serial.println(F("Start HTTP POST Data..."));
  wdt_reset();
  // Do HTTP POS"T communication with 10s for the timeout (read and write)
  rc = sim800l->doPost(URL2, CONTENT_TYPE, data, 20000, 20000);
  wdt_enable(WDTO_8S);
  if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST data successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received Serial: "));
    //Serial.println(sim800l->getDataReceived());
       } 
  else {
    // Failed...
    Serial.print(F("HTTP POST data error "));
    Serial.println(rc);
      }

  EEPROM.write(3,0);
  EEPROM.write(4,0);
  EEPROM.write(5,0);
  EEPROM.write(7,0);
// send data from sd card of a given day
  sendsd();
  wdt_disable();

  Serial.print("resetting");
  wdt_enable(WDTO_8S);
  delay(9000);     
}
  

void connect_check(){
  wdt_disable();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
     delay(1000);
     connected = sim800l->connectGPRS();
       }
  if(connected) {
     Serial.print(F("GPRS connected with IP "));
     Serial.println(sim800l->getIP());
       } 
  else{
     Serial.println(F("GPRS not connected !"));
     Serial.println(F("Reset the module."));
     sim800l->reset();
     setupModule();
     wdt_reset();
     return;
      }
    
}


void sendsd(void)
{
 myfile = SD.open(sdrequest, FILE_READ);
 while (myfile.available() )
 {
   String line = myfile.readStringUntil('\n'); // read a line from the file
   line.toCharArray(data,400);
   // Serial.println(data);
   wdt_enable(WDTO_8S);
   Serial.println(F("Start HTTP POST Data..."));
   // Do HTTP POS"T communication with 10s for the timeout (read and write)
   rc = sim800l->doPost(URL2, CONTENT_TYPE, data, 20000, 20000);

   if(rc == 200) {
     // Success, output the data received on the serial
     Serial.print(F("HTTP POST data successful ("));
     Serial.print(sim800l->getDataSizeReceived());
     Serial.println(F(" bytes)"));
     Serial.print(F("Received Serial : "));
     //Serial.println(sim800l->getDataReceived());
          } 
   else {
     // Failed...
     Serial.print(F("HTTP POST data error "));
     Serial.println(rc);
         }

   wdt_disable();
 }
 closeFile();
}

void emergency_send(){
  Serial.println("Collect Data...");

  now = rtc.now();  // Read data from the RTC Chip
  D =  String(now.year(), DEC) + "-" + String (now.month(), DEC) + "-" + String (now.day(), DEC);
  T = String(now.hour(), DEC) + ":" + String( now.minute(), DEC);
  D.toCharArray(Date, 11);
  T.toCharArray(Time, 6);
  Serial.println(Date);
  Serial.println(Time);
  
  la.toCharArray(lat, 10);
  lo.toCharArray(lon, 10);
  Serial.println("location array char:"); 
  Serial.println(lat);
  Serial.println(lon);
  delay(1000);
  
  sprintf(data, "{\"serialNumber\":%s,\"date\":\"%s %s\",\"lat\":\"%s\",\"long\":\"%s\"}"
          , Serialnumber, Date,Time, lat, lon);
  
  Serial.println(data);
  Serial.println(F("Start emergency POST Data..."));
  // Do HTTP POS"T communication with 10s for the timeout (read and write)
  rc = sim800l->doPost(URL3, CONTENT_TYPE, data, 20000, 20000);

  if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST data successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received Serial: "));
    //Serial.println(sim800l->getDataReceived());
        } 
  else{
    // Failed...
    Serial.print(F("HTTP POST data error "));
    Serial.println(rc);
        }
  }
