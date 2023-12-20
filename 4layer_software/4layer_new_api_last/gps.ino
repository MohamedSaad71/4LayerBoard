void GPS_run()
{

// This sketch displays information every time a new sentence is correctly encoded.
  for(int i = 0;i<=300;i++){
 while (Serial3.available()) { // check for gps data
      if (gps.encode(Serial3.read())) // encode gps data
      {
        lati = gps.location.lat();
        loni = gps.location.lng();
      //  Serial.println("Position: ");

        //Latitude
     //   Serial.print("Latitude: ");
        la = String(lati, 6);
      //  Serial.print(la);

     //   Serial.print(",");

        //Longitude
      //  Serial.print("Longitude: ");
        lo = String(loni, 6);
     //   Serial.println(lo);
      }
 
    }
    delay(10);
  }
}
