

#include <Ethernet.h>
#include <Wire.h>
#include <TLC591x.h>
#include <PWM.h>
#include <TimerOne.h>
#include <avr/wdt.h>


#define Send_Pin 13   /*the actual pin connected to atmega328 is 19*/  /*A6 Arduino*/
#define Buzzer_Pin 15  
#define Wind_Pin   17  

// Constructor parameters: # of chips, SDI in, CLK pin, LE pin, OE pin (optional parameter)
TLC591x myLED(1,5,10,8,9);
const byte interruptPin = 2;
int counter = 0;
int i =0;
int32_t frequency = 400000;
int classify;
int flag1 = 0;
unsigned long stateTime1 = 0; 
unsigned long stateTime2 = 0;


String DA = "";
char Send[80] = {0};
/**********************************************************************/
/*This value is set only at the beginning of the project only once*/
int Reference_value=925;  /* This value is the refrence value of the receivers */
int voltage = 110;
/*********************************************************************/
int Reff;
int ref1;
int flag=1;
int obstacle_flag;
int wind_value;
int Counter_Set = 0;
int Reset = 0;
int Send_Flag=0;
/************************* Variables to read the analog values to classify ****************************/
int size1;
int small;
int large;
int fly;
int mosquito;
char Receive;
/*****************************************************************************************************/


void setup()
{

  
  Wire.begin();
  pinMode(Send_Pin,INPUT);
  digitalWrite(Send_Pin,LOW);
  pinMode(Buzzer_Pin,OUTPUT);
  pinMode(Wind_Pin,INPUT);
  Serial.begin(9600);
  SetPinFrequencySafe(9, frequency);
  myLED.print(0x000000FF);
  analogWrite(9,voltage);
  delay(2000);
  set_ref();
  buzzer_off();  
  
 // set up Timer0 with a prescaler of 64
  TCCR0A = 0;
  TCCR0B = (1 << CS01) | (1 << CS00); // prescaler = 64

  // set up Timer0 to generate an interrupt every 10 milliseconds
  OCR0A = 4999; // 200ms / (1 / (16MHz / 64)) - 1 = 155.625
  TIMSK0 |= (1 << OCIE0A); // enable Timer0 compare match A interrupt
  
}

void loop()
{
//  Serial.print("Reference: ");
//  Serial.println(analogRead(A2));
//  delay(500);

//  Serial.print("Reff:");
//  Serial.println(analogRead(A2));
  delay(1000);
  if(Serial.available()>0)
  {
    Receive = Serial.read();
  }
  
  if(Receive == 'E')
  {
    buzzer_on();
  }
  if(Receive == 'S')
  {
    Send_Flag = 1;
    Receive = ' ';
  }
  
  if(Reset >= 3)
  {
    Serial.println("Resetting");
    wdt_enable(WDTO_4S);
    Reset = 0;
    delay(5000);
  }
  
  //Serial.println(digitalRead(13));
  if(Send_Flag)
  {
    Send_Flag = 0;
    Reset++;
    buzzer_off();
    wind_value = analogRead(Wind_Pin);
    int difference = Reference_value - Reff;
        if(difference >150)
        {
          obstacle_flag = 1;  /*This is the Indication to if there is an obstacle at the counter */
        }
    if(small == 1)
    {
      small = 0;    
    }
    DA = String(small) + "," + String(mosquito) + "," + String(fly) + "," + String(large) + "," + String(obstacle_flag)+ ","+ String(wind_value)+ ","+ "}";
    DA.toCharArray(Send, 80);
    Serial.println("Sending..");
    Serial.print("Small= ");
    Serial.println(small);
    Serial.print("Meduim= ");
    Serial.println(mosquito);
    Serial.print("Fly= ");
    Serial.println(fly);
    Serial.print("Large= ");
    Serial.println(large);
    Serial.print("Obstacle Flag: ");
    Serial.println(obstacle_flag);
    Serial.print("Wind Value: ");
    Serial.println(wind_value);
    Wire.beginTransmission(8); /* begin with device address 8 */
    Wire.write(Send);  
    Wire.endTransmission();    /* stop transmitting */
      
    small = 0;
    mosquito = 0;
    large = 0;
    fly = 0;
    counter = 0;
    obstacle_flag = 0;
    
    delay(2000);
  }
  
  
  
  
}


ISR(TIMER0_COMPA_vect)
{
  int temp;
  int refrence;
  // this function will be called every 10 milliseconds
  static int prevAnalogValue1 = 0;
  int currentAnalogValue1 = analogRead(A2);
  int smoothedValue1 = (prevAnalogValue1 + currentAnalogValue1) / 2;
  prevAnalogValue1 = currentAnalogValue1;
  int diff1 = smoothedValue1 - ref1;
  
  if ((diff1 < -1) && millis() - stateTime1 >= 10)
  {   
    for(int i=0;i<100;i++)
    {
      temp = analogRead(A2);
      if(temp>refrence)
      {
        refrence = temp;
      }
    }
  if (flag1 == 0)
  {
    counter++;
    flag1 = 1;
    size1 = analogRead(A2);
    classify = ref1-size1;
        
    Serial.print("Size: ");
    Serial.println(analogRead(A2));
    Serial.print("Reff: ");
    Serial.println(ref1);
    Serial.print("Classify: ");
    Serial.println(classify);
        
     if(classify <= 0)
     {
       small++;
     }
     else if (classify > 0 && classify <= 2 )
     {
       mosquito++;
     }
     else if (classify > 2 && classify <= 4)
     {
       fly++;
     }
     else
     {
       large++; 
     }
     if(Counter_Set == 1)
     {
       counter = 0;
       small = 0;
       Counter_Set = 0;
     }
          
   stateTime1 = millis();// increase delay to 500ms
   }

     
     Serial.print("Counter=");
     Serial.print(counter);
     Serial.print(",");
     Serial.print("s=");
     Serial.print(small); 
     Serial.print(",");
     Serial.print("m=");
     Serial.print(mosquito);
     Serial.print(",");
     Serial.print("F=");
     Serial.print(fly);
     Serial.print(",");
     Serial.print("l=");
     Serial.print(large);
     Serial.println("");
   }
  else
   {
    flag1 = 0;
   }
  
}


void set_ref()
{

  Counter_Set = 1;
  Serial.print("Original Reff:");
  Serial.println(analogRead(A2));
  while(analogRead(A2)<940 || analogRead(A2)>960)
  {
    if(analogRead(A2) >960)
   {
    voltage += 2;
    analogWrite(9,voltage);      // Control Power 
    delay(5);
   }
  else if(analogRead(A2) <940)
   {
    voltage -= 2;
    analogWrite(9,voltage);      // Control Power 
    delay(5);
   }
  }
 
  ref1=analogRead(A2);
  while(millis() < 40000)
  {
    
  if(analogRead(A2) < ref1)
  {
      ref1 = ref1-2;
  }
  Serial.print("ref1=");
  Serial.println(ref1);
  Serial.print("real=");
  Serial.println(analogRead(A2));

  }
  Serial.println("Done");
  ref1 = ref1-2;
  Reff = ref1; /*This is the reference taken after the reset */
  counter = 0;
}

void buzzer_on()
{
    digitalWrite(Buzzer_Pin,HIGH);
    delay(5000);
    digitalWrite(Buzzer_Pin,LOW);
    delay(5000);
} 

void buzzer_off()
{
   digitalWrite(Buzzer_Pin,LOW);
}
