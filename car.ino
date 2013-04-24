volatile unsigned long tick=0,tock=0;
volatile double speed = 0, speed_average = 0, speed_internal_average = 0;
volatile unsigned int ticks;
#define edges_per_revolution 8.0
#define revolutions_per_mile 2240.896358543417 // todo: Measure exact tire diameter (this assumes 9 in)
#define microseconds_per_hour 3600000000.0
#define ticks_per_update 10000

#define pin_thermistor1 0 //must be pin w/ ADC capability
#define pin_encoder 1 //must be pin w/external interrupt capability


void setup(){
  Serial.begin(9600);
  attachInterrupt(pin_encoder, speed_isr, RISING); // both edges would be a 40KHz signal @ 20mph @ 9inch diameter tires (16 edges per revolution)
}

void loop(){
  Serial.print("Sensor 1 Temperature = ");
  Serial.print(thermistor(pin_thermistor1));
  Serial.println(" F ");

  Serial.print("MPH = ");
  Serial.println(speed_average);
  
  delay(1000);
}


float thermistor(int pin){
    float resistance = 500.0/(1023.0/analogRead(pin) - 1);
    
    //pretty sure beta coefficient (3468) is wrong for my device, I need to grab some equipment to calculate it
    float steinhart_output = 1.0/(log(resistance/500.0)/3468 + (1.0 / (25.0+273.15)));
    float fahrenheit = (steinhart_output - 273.15) * 1.8 + 32.0;
    
    
    return fahrenheit;
}

void speed_isr(){
  tick = micros();
  speed = 1.0/((tick - tock) * edges_per_revolution / microseconds_per_hour * revolutions_per_mile);
  
  if(++ticks > ticks_per_update){
     ticks=1;
     speed_average = speed_internal_average/ticks_per_update;
     speed_internal_average=speed;
  }
  else{
   speed_internal_average+=speed;
  }
  
  tock = tick;
}
