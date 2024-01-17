void RTC_init()
{

  // Uncomment the following line to set the time
   rtc.adjust(DateTime(__DATE__, __TIME__));

  // If the RTC has lost power and is not running, set the time to the following line
  /*This line of code is configured only at the beginning of the code and then it's removed so the RTC keep it's time updated and not to set it every single time */
   //rtc.adjust(DateTime(2023, 6, 13, 11, 25, 15));

  // Display the current time
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
}

void RTC_run()
{

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000);
}
