void InitializeSDcard()
{
  Serial.println("Opening SD Card . . .");
  delay(500);
  if(SD.begin())
  {
    Serial.println("SD Card ready to use");
  }else{
    Serial.println("Failed to open SD Card");
    return;
  }
}

int createFile(char filename[])
{
  myfile = SD.open(filename, FILE_WRITE);

  if (myfile)
  {
    Serial.println("File created successfully.");
    return 1;
  } else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}


int OpenFile(char Filename[])
{
  myfile = SD.open(Filename, FILE_READ);
  delay(500);
  if(myfile)
  {
    Serial.println("File is open");
    return 1;
  }else{
    Serial.println("Error opening file");
    return 0;
  }
}


void closeFile()
{
  if (myfile)
  {
    myfile.close();
    Serial.println("File closed");
  }
}



int WriteToFile(char text[])
{
  if (myfile)
  {
    myfile.println(text);
    //Serial.println("the text is written successfully");
    return 1;
  }
  else
  {
    Serial.println("Couldn't write to file");
    return 0;
  }
}

/*Befor Reading from the file either using readfile() function or readline() function you have to use first the command
myfile = SD.open("BBBB.txt", FILE_READ);
then open, read and close
*/
void readFile(void)
{
    
    while (myfile.available())
    {
        char c = myfile.read(); // read a character from the file
        Serial.print(c);
    }
    
}
