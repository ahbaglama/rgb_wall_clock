#include <Keypad.h>
#include <virtuabotixRTC.h>
#include <Servo.h>
#include <FastLED.h>
#define LED_TYPE    WS2811
#define NUM_LEDS 60
#define DATA_PIN 3
#define CLOCK_PIN 13

int angle = 0;
int buzzerPin = 22;
int ledPower = 3;
int light_density = 0;
int light_value = 0;
int num = 0;
bool clockSettingMode = false; // Flag to indicate if clock setting mode is active
String newTime = ""; // Stores the new time input
// Define the keypad layout
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};



byte rowPins[ROWS] = {49, 48, 47, 46}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {45, 44, 43, 42}; //connect to the column pinouts of the keypad

//Create an object of keypad 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Define the clock module
virtuabotixRTC myRTC(10, 11, 12); // RTC object with CLK, DAT, and RST pins

//Create an object of servo
Servo servo;

CRGB leds[NUM_LEDS];



void setup() {
  Serial.begin(9600);
  servo.attach(8);
  servo.write(angle);
  myRTC.setDS1302Time(11, 11, 11, 2, 2, 1, 2023); // Set initial time and date (HH, MM, SS, DD, MM, YY, YYYY) 
  pinMode(buzzerPin,OUTPUT);
  pinMode(ledPower,OUTPUT);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  brightness();
  myRTC.updateTime();
  day_display();
  
  led_strip();
  //clock();//causes flickering across board, for debugging purpose only

  //hourly chime function, changing it's location may cause unwanted beeping
  if ((myRTC.minutes == 0) && (myRTC.seconds == 0)){ 
    myRTC.updateTime();
    delay(1000);
    digitalWrite(buzzerPin,HIGH); 
    delay(125);                  
    digitalWrite(buzzerPin,LOW);
    delay(125);
    digitalWrite(buzzerPin,HIGH);
    delay(125);
    digitalWrite(buzzerPin,LOW);
  }

  char key = keypad.getKey(); // Check if a key is pressed
  if (key) {
  buzzer();
  if (!clockSettingMode && key == 'A') {
      // Read the current time from the clock module
      myRTC.updateTime();

      // Print the current time
      Serial.print("Current time: ");
      Serial.print(myRTC.hours);
      Serial.print(":");
      Serial.print(myRTC.minutes);
      Serial.print(":");
      Serial.println(myRTC.seconds);
      
    } else if (!clockSettingMode && key == 'B') {
      // Enter clock setting mode
      clockSettingMode = true;
      newTime = ""; // Clear the new time input
      Serial.println("Enter new time (HHMMSSD)");
    } else if (!clockSettingMode && key == 'C') {
      // Change to different time zones
      //changeTimeZone();
    } else if (clockSettingMode) {
      // Handle clock setting mode
      if (key == 'B') {
        if (newTime.length() == 0) {
          // Exit clock setting mode if no new time is entered
          clockSettingMode = false;
          Serial.println("Clock setting mode exited without updating time");
        } else if (newTime.length() == 7) {
          // Exit clock setting mode and set the new time
          clockSettingMode = false;
          
          // Extract the day of week,bhours, minutes, and seconds from the input
          int newHours = newTime.substring(0, 2).toInt();
          int newMinutes = newTime.substring(2, 4).toInt();
          int newSeconds = newTime.substring(4, 6).toInt();
          int newDayofweek = newTime.substring(6,7).toInt();
          
          // Set the new time on the clock module
          myRTC.setDS1302Time(newSeconds, newMinutes, newHours, newDayofweek,myRTC.dayofmonth, myRTC.month, myRTC.year);

          // Print the updated time
          Serial.print("Updated time: ");
          Serial.print(newHours);
          Serial.print(":");
          Serial.print(newMinutes);
          Serial.print(":");
          Serial.println(newSeconds);
          Serial.println(":");
          Serial.println(newDayofweek);
          Serial.println(":");

        } else {
          // Invalid time input
          Serial.println("Invalid time input");
        }
        
        newTime = ""; // Clear the new time input
      } else {
        // Append the entered key to the new time input
        newTime += key;
        Serial.print(key);
      }
    }
  }
}


void brightness(){
  light_density = analogRead(A0);//reading from light sensor
  light_value = map(light_density,0,1023,0,255);//analog to pwm conversion
  
  //auto brightness
  if (light_density <150){
    FastLED.setBrightness(light_value);
  }
  else if ((light_density >400) && (light_value <600)){
    FastLED.setBrightness(light_value);
  }
  else if ((light_density >600) && (light_value <800)){
    FastLED.setBrightness(light_value);
  }
  else if (light_density >800){
    FastLED.setBrightness(light_value);
  }
}

void buzzer(){
  myRTC.updateTime();
  digitalWrite(buzzerPin,HIGH); 
  delay(125);                  
  digitalWrite(buzzerPin,LOW);
  delay(125);
  FastLED.clear();
}

void day_display(){
  
  if (myRTC.dayofweek == 7){
    servo.write(0);
  }
  if (myRTC.dayofweek == 6){
    servo.write(12);
  }  
  if (myRTC.dayofweek == 5){
    servo.write(37);
  }  
  if (myRTC.dayofweek == 4){
    servo.write(65);
  }  
  if (myRTC.dayofweek == 3){
    servo.write(90);
  }  
  if (myRTC.dayofweek == 2){
    servo.write(115);
  }  
  if (myRTC.dayofweek == 1){
    servo.write(140); 
  }  

}

void clock(){ //for debugging only
  int hour = myRTC.hours;
  int minute = myRTC.minutes;
  int second = myRTC.seconds;
  Serial.print("Hour: ");
  Serial.print(hour);
  Serial.print(" Minute: ");
  Serial.print(minute);
  Serial.print(" Second: ");
  Serial.println(second);
}

void led_strip(){   
  //myRTC.updateTime();
  int hour = myRTC.hours;
  int minute = myRTC.minutes;
  int second = myRTC.seconds;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Set the LED color based on the current time
  if (second < NUM_LEDS) {
    leds[second] = CRGB::Blue;
  }

  if (minute < NUM_LEDS) {
    leds[minute] = CRGB::Green;
  }

  if (hour < NUM_LEDS) {
    leds[(hour*5)%60] = CRGB::Red;
  }

  // Display the LEDs
  FastLED.show();    
      
}
