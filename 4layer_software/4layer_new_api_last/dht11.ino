void DHT11_init()
{
    Serial.println(F("DHT22 Begin"));

    dht.begin();
}

void DHT11_run()
{

   delay(1000); // Wait for 1 seconds
 float temperature = dht.readTemperature(); // Read temperature value
 float humidity = dht.readHumidity(); // Read humidity value

  // Check if any errors occurred during reading
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error reading data from DHT22 sensor");
    return;
  }

  // Display the temperature and humidity values on the serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}
