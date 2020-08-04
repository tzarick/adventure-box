//#include <Adafruit_SSD1306.h>
//#include <splash.h>
//
//#include <Adafruit_GFX.h>
//#include <Adafruit_SPITFT.h>
//#include <Adafruit_SPITFT_Macros.h>
//#include <gfxfont.h>
#include <EEPROM.h>

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
#include <SPI.h>
#include <Wire.h>


#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

// chest sound params
int speakerPin = 3;
const int switchPin = 1;
const int buttonPin = 7;
const int fixLed = 2;

char notes[] = "gabygabyxzCDxzCDabywabywzCDEzCDEbywFCDEqywFGDEqi        azbC"; // a space represents a rest
int length = sizeof(notes); // the number of notes
int beats[] = { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 2,3,3,16,};
int tempo = 75;

char notes_poke_heal[] = "bbbgG";
int length_poke = sizeof(notes_poke_heal);
int beats_poke_heal[] = { 4,4,2,2,3 };
int tempo_poke = 80;

enum {
  ACQUIRING,
  RLY_FAR,
  FAR,
  CLOSER,
  ON_FIRE,
  ARRIVED
};

enum { // journey_status
  XMAS_MORN,
  NO_PROGRESS,
  CHECKPOINT_ARRIVED,
  CHECKPOINT_COMPLETED,
  DESTINATION_ARRIVED
};


int cur_status = ACQUIRING;

int journey_status = EEPROM.read(0); // last session's arrival status is saved in addr 0 of EEPROM --> already fully completed journey if this loc contains DESTINATION_ARRRIVED (3)

// gps setup
SoftwareSerial ss(4,5);
//AltSoftSerial altSerial;
TinyGPSPlus gps;

// display
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display

// goh: 40.018109, -83.011569
// 14th ave: 39.998331, -83.0008302
// griz tree: 37.5036948, -119.6014831
// cleveland sign: 41.487578, -81.749272
// Terrapin Point: 43.080044,-79.0744102
// Sapphire Point: 39.588462, -106.044168
// cottell: 39.305712, -84.331156
// 9331: 39.302457, -84.343694
const double checkpointTarget[2] = {40.018109, -83.011569};
const double finalTarget[2] = {39.588462, -106.044168};
bool flag_checkpoint_just_completed = false;
double targetLat;
double targetLng;
#define milesInKm 0.62137119223
#define feetInMile 5280.0
#define COMBINATION 507

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'x', 'a', 'z', 'b', 'C', 'y', 'D', 'w', 'E', 'F', 'q', 'G', 'i' };
  // c=C4, C = C5. These values have been tuned.
  int tones[] = { 1898, 1690, 1500, 1420, 1265, 1194, 1126, 1063, 1001, 947, 893, 843, 795, 749, 710, 668, 630, 594 };
   
  // play the tone corresponding to the note name
  for (int i = 0; i < 18; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playChestSound() {
    if (digitalRead(switchPin) == 1) {
      for (int i = 0; i < length; i++) {
        if (notes[i] == ' ') {
          delay(beats[i] * tempo); // rest
        } else {
          playNote(notes[i], beats[i] * tempo);
        }
          
        // pause between notes
        delay(tempo / 2); 
    }
  }
}

void playPokeSound() {
    if (digitalRead(switchPin) == 1) {
    for (int i = 0; i < length_poke; i++) {
      if (notes_poke_heal[i] == ' ') {
        delay(beats_poke_heal[i] * tempo); // rest
      } else {
        playNote(notes_poke_heal[i], beats_poke_heal[i] * tempo_poke);
      }
        
      // pause between notes
      delay(tempo_poke / 2); 
    }
  }
}


void arrivalRoutine() {
  playChestSound();
  digitalWrite(fixLed, LOW);
  // display combination
  u8x8.setCursor(0,4);
  u8x8.print("Code:");
  u8x8.setCursor(6,4);
  u8x8.print(COMBINATION);
}


void setup() {
//  EEPROM.write(0,0);
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  

  if (journey_status != DESTINATION_ARRIVED) {
      Serial.begin(9600);
    //  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    //    Serial.println(F("SSD1306 display screen allocation failed"));
    //  }
      Serial.println("start");
      targetLat = (journey_status >= CHECKPOINT_COMPLETED) ? finalTarget[0] : checkpointTarget[0];
      targetLng = (journey_status >= CHECKPOINT_COMPLETED) ? finalTarget[1] : checkpointTarget[1];

    //  altSerial.begin(9600);
      ss.begin(9600);

       if (journey_status == XMAS_MORN) {
          u8x8.print("Merry X-mas");
          u8x8.setCursor(0,1);
          u8x8.print("Nicky & Liv!! :)");
          u8x8.setCursor(0,4);
          u8x8.print("Love, Tommy");
       }
       else if (journey_status == CHECKPOINT_ARRIVED) { // arrived at checkpoint but haven't completed it yet
          u8x8.drawString(0,0,"Acquiring...");
          u8x8.setCursor(0,4);
          u8x8.print("You made it!");
          u8x8.setCursor(0,5);
          u8x8.setInverseFont(1);
          u8x8.print("go down ravine.");
          u8x8.setInverseFont(0);
          u8x8.setCursor(0,6);
          u8x8.print("Enter the mouth");
          u8x8.setCursor(0,7);
          u8x8.print("50 paces 4 hint");
      }
      else {
        u8x8.drawString(0,0,"Acquiring...");
      }
      
      u8x8.refreshDisplay();  
    
    
      pinMode(speakerPin, OUTPUT);
      pinMode(switchPin, INPUT);
      pinMode(fixLed, OUTPUT);
      digitalWrite(fixLed, LOW);
      digitalWrite(switchPin, HIGH);
  }
  else { // game over here on out

      u8x8.print("You rock!");
      u8x8.setCursor(0,2);
      u8x8.print("Merry Xmas :)");
      u8x8.setCursor(0,4);
      u8x8.print("Code:");
      u8x8.setCursor(6,4);
      u8x8.print(COMBINATION);
      while(1); // nothing forever
  }


}

void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(fixLed, LOW);
    
    if (journey_status == XMAS_MORN && digitalRead(buttonPin) == HIGH) {
      journey_status = NO_PROGRESS;
      EEPROM.write(0,journey_status);
      u8x8.clear();
      u8x8.drawString(0,0,"Acquiring...");
      u8x8.refreshDisplay();
    }
    
    unsigned long button_press_instant = millis(); 
    while (journey_status == CHECKPOINT_ARRIVED && digitalRead(buttonPin) == HIGH) {
      if (millis() - button_press_instant > 7000) { // checkpoint complete!!
        ss.end(); // stop bc of interference with pwm/buzzer
        playPokeSound();
        ss.begin(9600);
        // update destination and journey status and screen
        journey_status = CHECKPOINT_COMPLETED;
        EEPROM.write(0, journey_status);
        flag_checkpoint_just_completed = true;
        targetLat = finalTarget[0];
        targetLng = finalTarget[1];
        u8x8.clearLine(4);
        u8x8.clearLine(5);
        u8x8.clearLine(6);
        u8x8.clearLine(7);
        u8x8.setCursor(0,4);
        u8x8.print("Nice work!u pass");
        u8x8.setCursor(0,5);
        u8x8.print("the checkpoint!!");
        u8x8.setCursor(0,6);
        u8x8.print("Next stop: ur");
        u8x8.setCursor(0,7);
        u8x8.print("actual dst!:)");
        u8x8.refreshDisplay();
      }
    }
    
    while (ss.available() > 0 && cur_status != ARRIVED && journey_status > XMAS_MORN){
     
    // get the byte data from the GPS
    byte gpsData = ss.read();
    gps.encode(gpsData);
    if (gps.location.isUpdated()){
      digitalWrite(fixLed, HIGH);
      Serial.println();
      double *coords = getLocation(); 
      Serial.println(coords[1], 6);
      Serial.println(coords[0], 6);
      double distance = distanceToTarget(coords[0], coords[1], targetLat, targetLng); 
      double distanceToX =
        TinyGPSPlus::distanceBetween(
          gps.location.lat(),
          gps.location.lng(),
          targetLat, 
          targetLng); // in meters
      double courseToX =
        TinyGPSPlus::courseTo(
          gps.location.lat(),
          gps.location.lng(),
          targetLat, 
          targetLng);
      Serial.print("distance:   ");
      Serial.println(distance, 6);
      double mi = kmToMiles(distanceToX/1000);
      Serial.println(mi, 6);
      double f = milesToFeet(mi);
      Serial.println(f, 6);
      Serial.print("distTo: ");
      Serial.println(distanceToX/1000, 6);
      Serial.print("course to: ");
      Serial.println(courseToX, 6);

      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.setCursor(0,0);
      u8x8.print("Distance away:");
      u8x8.setCursor(0,1);
      u8x8.print(mi, 6);
      u8x8.print(" mi");
      u8x8.setCursor(0,2);
      u8x8.print("Head:");
      u8x8.setCursor(0,3);
      u8x8.print(courseToX, 6);
      u8x8.print(" deg");
      u8x8.refreshDisplay(); 
      
      if (f < 300) {
        // congrats and display code
        if (journey_status == NO_PROGRESS) { // arrived at checkpoint but haven't completed it yet
          // update journey status and screen with extra hint
          u8x8.clearLine(4);
          u8x8.clearLine(5);
          u8x8.clearLine(6);
          u8x8.clearLine(7);
          u8x8.setCursor(0,4);
          u8x8.print("You made it!");
          u8x8.setCursor(0,5);
          u8x8.print("go down ravine.");
          u8x8.setCursor(0,6);
          u8x8.print("Enter the mouth");
          u8x8.setCursor(0,7);
          u8x8.print("50 paces 4 hint");
          u8x8.refreshDisplay();
          journey_status = CHECKPOINT_ARRIVED;
          EEPROM.write(0, journey_status);
        }
        else if (journey_status == CHECKPOINT_COMPLETED){ // arrival at final target destination
          u8x8.clear();
          u8x8.print("Arrived!!!! :D");
          u8x8.setCursor(0,1);
          u8x8.print("CONGRATS");
          u8x8.setCursor(0,2);
          u8x8.print("NICKY & LIV!!!");
          u8x8.refreshDisplay();
          ss.end();
          arrivalRoutine();
          cur_status = ARRIVED;
          EEPROM.write(0, DESTINATION_ARRIVED); // point of no return. Everything is different now
        }

      }
      else if (mi < 1) {
        if (cur_status != ON_FIRE) {
          if (journey_status >= CHECKPOINT_COMPLETED) {
            u8x8.clearLine(5);
            u8x8.setCursor(0,5);
            u8x8.print("Good job so far!");
          }
          u8x8.clearLine(6);
          u8x8.setCursor(0,6);
          u8x8.print("You're on fire!!");
          u8x8.setCursor(0,7);
          u8x8.print("Sooooo close...");
          u8x8.refreshDisplay();
          cur_status = ON_FIRE;
        }
      }
      else if (mi < 10) {
        if (cur_status != CLOSER) {
          if (journey_status >= CHECKPOINT_COMPLETED) {
            u8x8.clearLine(5);
            u8x8.setCursor(0,5);
            u8x8.print("Good job so far!");
          }
          u8x8.clearLine(6);
          u8x8.clearLine(7);
          u8x8.setCursor(0,6);
          u8x8.print("Can you smell it");
          u8x8.setCursor(0,7);
          u8x8.print("????????");
          u8x8.refreshDisplay();
          cur_status = CLOSER;
        }
      }
      else if (mi < 100) {
        if (cur_status != FAR) {
          if (journey_status >= CHECKPOINT_COMPLETED) {
            u8x8.clearLine(5);
            u8x8.setCursor(0,5);
            u8x8.print("Good job so far!");
          }
          u8x8.clearLine(6);
          u8x8.clearLine(7);
          u8x8.setCursor(0,6);
          u8x8.print("Getting warmer.");
          u8x8.setCursor(0,7);
          u8x8.print("Keep it up! :)");
          u8x8.refreshDisplay();
          cur_status = FAR;
        }
      }
      else {
        if (cur_status != RLY_FAR) {
          if (!flag_checkpoint_just_completed) {
            if (journey_status >= CHECKPOINT_COMPLETED) {
              u8x8.clearLine(5);
              u8x8.setCursor(0,5);
              u8x8.print("Good job so far!");
            }
            u8x8.clearLine(6);
            u8x8.clearLine(7);
            u8x8.setCursor(0,6);
            u8x8.print("Pretty far away.");
            u8x8.setCursor(0,7);
            u8x8.print("Get moving! :0");
            u8x8.refreshDisplay();
          }
            cur_status = RLY_FAR; 
        }
      }
    }
    Serial.write(gpsData);
  }
}

double * getLocation() {
  static double loc[2];

  loc[0] = gps.location.lat();
  loc[1] = gps.location.lng();
  
  return loc;
}

double milesToFeet(double mi) {
  return mi * feetInMile;
}

double kmToMiles(double km) {
  return km * milesInKm;
}

// convert degrees to radians
double degToRad(double degree) {
  return degree * PI/180;
}

// distance between 2 geographical coordinates in km
double distanceToTarget(double lat1, double lng1, double lat2, double lng2) {
  double earthRadius = 6371; // m
  
  double distLat = degToRad(lat2 - lat1);
  double distLng = degToRad(lng2 - lng1);
  
  double lat1_rad = degToRad(lat1);
  double lat2_rad = degToRad(lat2);

  double a = sin(distLat / 2) * sin(distLat / 2) + cos(lat1_rad) * cos(lat2_rad) * sin(distLng / 2) * sin(distLng / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));

  return earthRadius * c;
}
