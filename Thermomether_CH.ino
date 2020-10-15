/****************************************************************
Thermomether Program v1.0
****************************************************************/
#include <SparkFun_APDS9960.h>
#include <Adafruit_MLX90614.h>
#include "U8glib.h"
#include "Thermomether_CH.h"

// OLED Display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
// Proximity Sensor
SparkFun_APDS9960 apds = SparkFun_APDS9960();
// Temperature Sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
// Ultrasonic Sensor
const uint8_t echoPin = 3;
const uint8_t trigPin = 4;
// Relay
const uint8_t relayPin = 7;
// Piezzo Buzzer
const uint8_t piezzoPin = 8;
// Global Vars
uint8_t proximityData = 0;
double averageTemp = 0.0;
long ultrasonicDuration;
int ultrasonicDistance;
int feverMelody[] = {NOTE_C4, NOTE_C4};
int farMelody[] = {NOTE_G3, NOTE_G3};
int noteDurations[] = {2, 2};

void draw(uint8_t mode) {
  // Draw texts.
  u8g.setPrintPos(20, 18);
  u8g.setFont(u8g_font_unifont);
  switch (mode) {
    case 0:
      u8g.print("MOVE CLOSER"); 
      u8g.drawStr(19, 40, "KURANG DEKAT");
      break;
    case 1:
      u8g.setFont(u8g_font_fur17);
      u8g.setPrintPos(17, 35);
      u8g.print(averageTemp); 
      u8g.print("*C");
      break;
    case 2:
      u8g.setPrintPos(9, 28);
      u8g.print("FEVER > 37.5*C");
      u8g.drawStr(9, 42, "DEMAM > 37.5*C");
      break;
    case 3:
      u8g.print("MOVE FURTHER");
      u8g.drawStr(19, 32, "MOHON MENJAUH");
      break;
  }
}

void tonePlayer(int mode) {
  switch (mode) {
    case 0:
      // One "teet" tone.
      noTone(5);
      tone(piezzoPin, NOTE_E7, 100);
      break;
    case 1:
      // Two "teet" tones.
      for (int thisNote = 0; thisNote < 2; thisNote++) {
          // to calculate the note duration, take one second divided by the note type.
          //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
          int noteDuration = 1000 / noteDurations[thisNote];
          tone(piezzoPin, feverMelody[thisNote], noteDuration);
          // to distinguish the notes, set a minimum time between them.
          // the note's duration + 30% seems to work well:
          int pauseBetweenNotes = noteDuration * 1.30;
          delay(pauseBetweenNotes);
          // stop the tone playing:
          noTone(8);
      }
      break;
    default:
      break;
    /*case 2:
      for (int thisNote = 0; thisNote < 2; thisNote++) {
          // to calculate the note duration, take one second divided by the note type.
          //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
          int noteDuration = 1000 / noteDurations[thisNote];
          tone(piezzoPin, farMelody[thisNote], noteDuration);
          // to distinguish the notes, set a minimum time between them.
          int pauseBetweenNotes = noteDuration;
          delay(pauseBetweenNotes);
          // stop the tone playing:
          noTone(8);
      }
      break;*/
     
  }
}

void findDistance() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  ultrasonicDuration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  ultrasonicDistance = ultrasonicDuration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(ultrasonicDistance);
}

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  delay(100);

  // Ultrasonic Sensor Initialization
  Serial.println();
  Serial.println(F("--------------------------------------------"));
  Serial.println(F("Ultrasonic - Distance Sensor: Initialization"));
  Serial.println(F("--------------------------------------------"));
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println(F("Ultrasonic Sensor initialization complete!"));

  // Relay Initialization
  Serial.println();
  Serial.println(F("---------------------"));
  Serial.println(F("Relay: Initialization"));
  Serial.println(F("---------------------"));
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); 
  
  if (digitalRead(relayPin))
    Serial.println(F("Relay initialization complete!"));
  else
    Serial.println(F("Relay initialization failed!"));

  // APDS-9960 Initialization
  Serial.println();
  Serial.println(F("--------------------------------------------"));
  Serial.println(F("APDS-9960 - Proximity Sensor: Initialization"));
  Serial.println(F("--------------------------------------------"));
  
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() )
    Serial.println(F("APDS-9960 initialization complete"));
  else
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  
  // Adjust the Proximity sensor gain
  if ( !apds.setProximityGain(PGAIN_2X) )
    Serial.println(F("Something went wrong trying to set PGAIN on APDS-9960"));
  
  // Start running the APDS-9960 proximity sensor (no interrupts)
  if ( apds.enableProximitySensor(false) )
    Serial.println(F("APDS-9960 initialization complete!"));
  else
    Serial.println(F("Something went wrong during sensor init!"));

  // MLX90614 Initialization
  Serial.println();
  Serial.println(F("--------------------------------------"));
  Serial.println(F("MLX90614 - Temp Sensor: Initialization"));
  Serial.println(F("--------------------------------------"));

  // Initialize MLX90614 (configure I2C and initial values)
  if ( mlx.begin() )
     Serial.println(F("MLX90614 initialization complete!"));
  else
    Serial.println(F("Something went wrong during MLX90614 init!"));

  // OLED Display Initialization
  Serial.println();
  Serial.println(F("------------------------------"));
  Serial.println(F("OLED - Display: Initialization"));
  Serial.println(F("------------------------------"));

  // White
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     
  }
  // Max Intensity
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3); 
  }
  // Pixel On
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);    
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  Serial.println(F("OLED Display initialization complete!"));
  
}

void doorOpener() {
  // Initialize frame for the OLED display.
  u8g.firstPage();

  // Assign proximity data to proximityData.
  apds.readProximity(proximityData);
  
  if ( proximityData == 255 ) {
      
      // For 10 times, get the temperature,
      for (int i = 0; i < 11; i++)
        averageTemp += mlx.readObjectTempC();
        
      // Then find the average.
      averageTemp /= 10;
      
      if ( averageTemp < 37.50 ) {
        do {
          draw(1);
        } while( u8g.nextPage() );
        tonePlayer(0);
        Serial.print("Body Temperature = "); Serial.print(averageTemp); Serial.println("°C");
        Serial.println();
        // Reset average temp every loop.
        averageTemp = 0.0;
        digitalWrite(relayPin, HIGH);
        delay(5000);
        digitalWrite(relayPin, LOW);
      } else if ( averageTemp >= 37.50 ) {
        do {
          draw(2);
        } while( u8g.nextPage() );
        tonePlayer(1);
        Serial.print("Fever Temperature: "); Serial.print(averageTemp); Serial.println("°C");
        Serial.println();
        averageTemp = 0.0;
        delay(5000);
      }
  } else {
      if ( !apds.readProximity(proximityData) ) {
        Serial.println("Error reading proximity value");
      } else {
        do {
          draw(0);
        } while( u8g.nextPage() );
        Serial.print("Move closer! Proximity: ");
        Serial.println(proximityData);
        averageTemp = 0.0;
        // Run objectDoorOpener here so that it's *visually* simultaneously running with doorOpener.
        objectDoorOpener();
      }
  }
}

void objectDoorOpener() {
  // Find distance from ultrasonic sensor.
  findDistance();
  // Check if an object is detected 6 cm and below.
  if (ultrasonicDistance != 0 && ultrasonicDistance < 11) {
    // Unlock if detected,
    digitalWrite(relayPin, HIGH);
    delay(3000);
  } else {
    // Lock if not.
    digitalWrite(relayPin, LOW);
    delay(3000);
  }
}


void loop() {
  doorOpener();
}
