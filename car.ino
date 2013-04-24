#include "PITimer.h"
#include <Servo.h>
#include <LiquidCrystal.h>

//speed calculation variables/constants
volatile unsigned long tick=0,tock=0;
volatile double speed = 0, speed_average = 0, speed_internal_average = 0;
volatile unsigned int ticks;
#define edges_per_revolution 8.0
#define revolutions_per_mile 2240.896358543417 // todo: Measure exact tire diameter (this assumes 9 in)
#define microseconds_per_hour 3600000000.0
#define ticks_per_update 10000

//windshield wiper variables/constants
Servo wiper;
volatile int position = 0;
volatile int direction = 1;

//pins
#define pin_thermistor1 0 //must be pin w/ ADC capability
#define pin_encoder 1 //must be pin w/external interrupt capability
#define pin_servo 2 //might need PWM pin
#define pin_lcd_rs 3
#define pin_lcd_en 4
#define pin_lcd_d4 8
#define pin_lcd_d5 7
#define pin_lcd_d6 6
#define pin_lcd_d7 5

LiquidCrystal lcd(pin_lcd_rs, pin_lcd_en, pin_lcd_d4, pin_lcd_d5, pin_lcd_d6, pin_lcd_d7);

void setup(){
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.print("BarbieOS Booting");
  delay(1000); //So we can enjoy the load message.
  lcd.setCursor(0,0);
  lcd.print("MPH  ETEMP BTEMP");
  
  attachInterrupt(pin_encoder, speed_isr, RISING); // both edges would be a 40KHz signal @ 20mph @ 9inch diameter tires (16 edges per revolution)

  wiper.attach(pin_servo);
  PITimer1.period(0.015); // 15ms
  PITimer1.start(wiper_isr);
}

void loop(){
  serial_json_log();
  lcd_update();
  
  delay(1000); //todo: increase speed after done debugging
}

void serial_json_log(){
  Serial.print("{\"engineTemp\": ");
  Serial.print(thermistor(pin_thermistor1), 4);
  Serial.print(", \"mph\": ");
  Serial.print(speed_average, 4);
  Serial.println(" }");
}

void lcd_update(){
  //Needs to fit nicely under "MPH  ETEMP BTEMP" line
  lcd.setCursor(0,1);
  lcd.print(speed_average, 4);
  lcd.setCursor(5,1);
  lcd.print(thermistor(pin_thermistor1), 4);
  lcd.setCursor(11,1);
  lcd.print("0.000"); //todo: add another thermistor
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

void wiper_isr(){
  if(position < 90 && position > 0){
    position+=direction;
    wiper.write(position);
  }
  else{
    direction*=-1;
    position+=direction; 
  }
}
