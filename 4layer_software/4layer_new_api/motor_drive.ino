void fan_init(){
  pinMode(en_fan, OUTPUT);
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  
  digitalWrite(en_fan, LOW);
  digitalWrite(fan1, LOW);
  digitalWrite(fan2, LOW);
  }
  void valve_init(){
  pinMode(en_valve, OUTPUT);
  pinMode(valve1, OUTPUT);
  pinMode(valve2, OUTPUT);
  
  digitalWrite(valve1, LOW);
  digitalWrite(valve2, LOW);
    }
void fan_start(int val){
  analogWrite(en_fan, val);
  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, LOW);
  }
  void valve_start(unsigned int val){
  if(val<valve_oldvalue){
  digitalWrite(valve1, HIGH);
  digitalWrite(valve2, LOW);
  analogWrite(en_valve, 255);
  delay(valve_oldvalue - val);
  analogWrite(en_valve,0);
    valve_oldvalue = val;
    }
    else if(val>valve_oldvalue){
  digitalWrite(valve1, LOW);
  digitalWrite(valve2, HIGH);
  analogWrite(en_valve, 255);
  delay(val-valve_oldvalue);
  analogWrite(en_valve,0);
  valve_oldvalue = val;
    }
    else{}
  }
  void fan_stop(){
  digitalWrite(en_fan, LOW);
   digitalWrite(fan1, LOW);
  digitalWrite(fan2, LOW);
    }
    
void valve_stop(){
  digitalWrite(en_valve, LOW);
  digitalWrite(valve1, LOW);
  digitalWrite(valve2, LOW); 
    }
