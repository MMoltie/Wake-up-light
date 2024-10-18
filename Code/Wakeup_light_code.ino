#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "DFRobot_SD3031.h"
#include <Encoder.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "Settings.h"
#include "BY8X01-16P.h"



/*----------PINOUT----------*/
//Display
#define TFT_DC 7
#define TFT_CS 10
#define TFT_RES 8

//RTC
//No pinouts defined

//Rotary encoder
#define RE_A 2
#define RE_B 3

//Soundboard
#define SFX_TX A3
#define SFX_RX 4
#define SFX_BUSY 12

//Buttons
#define setButton A1
#define alarmButton A0

//LEDs
#define blueLED 5
#define purpleLED 6
#define orangeLED 9



/*----------Component initialization----------*/
//Display
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RES);
int framecount = 0; //variable to keep track of display update periods
int dimcount = 0;

//RTC
DFRobot_SD3031 rtc;

//Rotary encoder
Encoder myEnc(RE_A, RE_B);
long position = 0;

//Soundboard
SoftwareSerial swSerial(SFX_RX, SFX_TX);
BY8X0116P audioController(swSerial);

//Buttons

//LEDs


/*----------Variables----------*/
//Counter variables copied from Generic_RTC_clock
unsigned long prev, interval = 100;     //variables for display and clock update rates (60000 = 1 minute)
int debounceDelay = 50;                 //Debounce for rotary encoder
unsigned long lastDebounceTime = 0;

byte mode = 0;                          //default clock mode
byte alarmCase = 0;                     //alarm case
int readingTime = 0;
float brightnessBlue, brightnessPurple, brightnessOrange, brightnessRL, brightnessLED;

uint8_t alarmSet;                       //Boolean for alarm on or off
uint16_t tempset;                       //Variable to store adjustable time
char timeHour[4];                       //A string to contain "hh" part of time
char timeMin[4];                        //A string to contain "mm" part of time
char alarmTimeHour[4];                  //A string to contain "hh" part of ALARM time
char alarmTimeMin[4];                    //A string to contain "mm" part of ALARM time

//DateTime variables for current time, alarm start time, alarm time and alarmstop time
sTimeData_t t, alarmStart, alarmTime, alarmStop;
unsigned long convertTimetoUnix, convertATtoUnix, alarmStartUnix, alarmStopUnix;
long secToRead, readEnd, dimFlag;



void setup() {
  Serial.begin(115200);

  while (!Serial){
    ; //wait for serial port to open
  }

  Serial.println("Serial communication started.");
  Serial.println("Wake-up light test sequence...");

  //Start display communication and setup
  tft.begin();
  tft.setTextColor(ColorOne, GC9A01A_BLACK);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setRotation(2);
  Serial.println("Display set");

  //Start RTC communication
  while(rtc.begin() != 0){
    Serial.println("Couldn't find RTC");
    delay(500);
  }

  rtc.setHourSystem(rtc.e24hours); //set display to 24hr format
  setCompileTime();
  Serial.println("RTC connected");

  //Start soundboard comm
  swSerial.begin(audioController.getSerialBaud()); //begin softwareserial
  while (!swSerial){
    ; //wait for serial port to open
  }
  Serial.println("soundboard connected");
  pinMode(SFX_RX, INPUT);
  pinMode(SFX_TX, OUTPUT);

  audioController.init();
  audioController.setVolume(25);

  //Connect buttons and LEDs
  Serial.println("Connecting Buttons and LEDs");
  pinMode(setButton, INPUT_PULLUP);
  pinMode(alarmButton, INPUT_PULLUP);
  pinMode(blueLED, OUTPUT);
  pinMode(purpleLED, OUTPUT);
  pinMode(orangeLED, OUTPUT);

  // Change Timer0 to increase frequency on pins 5 and 6
  TCCR0B = TCCR0B & 0b11111000 | 0x01;  // Set prescaler to 1, frequency = ~62500Hz

  //LEDs check
  for(int i = 0; i<=255; i+=5){
    analogWrite(blueLED, i);
    delay(10);
  }
  delay(10);
  for(int i = 0; i<=255; i+=5){
    analogWrite(purpleLED, i);
    delay(10);
  }
  for(int i = 255; i>=0; i-=5){
    analogWrite(blueLED, i);
    delay(10);
  } 
  delay(10);  
  for(int i = 0; i<=255; i+=5){
    analogWrite(orangeLED, i);
    delay(10);
  }
  for(int i = 255; i>=0; i-=5){
    analogWrite(purpleLED, i);
    delay(10);
  } 
  delay(10); 
  for(int i = 255; i>=0; i-=5){
    analogWrite(orangeLED, i);
    delay(10);
  }

  Serial.println("Setup complete!");
}

void loop() {
  unsigned long now = millis();
  long newPos = myEnc.read(); 

  if (now - prev > interval){
    framecount = framecount + 1;
    dimcount = dimcount + 1;
    
    //if enough time has passed, reset the mode to 0 and restart framecount timer
    if (framecount > 20){
      framecount = 0;
      mode = 0;
    }

    //get the current time
    t = rtc.getRTCTime();

    getAlarm();

    updateLights();

    updateSound();

    updateDisplay();

    switch(mode){
      case 0: //default 
        if (newPos != position){
          dimFlag = millis() + 3600000; //set a flag in 1 seconds
          dimcount = 0;

          if(newPos >= position){
            if(readingTime <= 5){
              readingTime = readingTime + 1;
            } else if (readingTime > 5 && readingTime <=30){
              readingTime = readingTime + 5;
            } else { readingTime = readingTime + 10;}
          } else {
            if(readingTime <= 5){
              readingTime = readingTime - 1;
            } else {readingTime = readingTime - 5;}
            if(readingTime < 0){
              readingTime = 0;
            }  
          }


          readEnd = convertTimetoUnix + (readingTime*60);  //reading end time is current time + readingtime (in Unix)
        }
      break;

      case 1: //Hour adjust
        tempset = t.hour;

        if (newPos != position && (millis() - lastDebounceTime) > debounceDelay){    //if the rotary encoder is turned..
          lastDebounceTime = millis();
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          
          if (newPos > position){
            tempset = (tempset + 1) % 24;
          } else {
            tempset = (tempset == 0) ? 23: tempset -1;
          }  
          
          rtc.setTime(t.year,t.month,t.day,tempset,t.minute,t.second);
          t = rtc.getRTCTime();
        }
      break;

      case 2: //Minute adjust
        tempset = t.minute;
        
        if (newPos != position){    //if the rotary encoder is turned..
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          
          if (newPos > position){
            tempset = (tempset + 1) % 60;
          } else {
            tempset = (tempset == 0) ? 59: tempset - 1;
          }  
          
          rtc.setTime(t.year,t.month,t.day,t.hour,tempset,t.second);
          t = rtc.getRTCTime();
        }
      break;

      case 3: //Alarm hour adjust
        tempset = wakeHour;
        
        if (newPos != position){    //if the rotary encoder is turned..
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          if (newPos > position){
            tempset = (tempset + 1) % 24;
          } else {
            tempset = (tempset == 0) ? 23: tempset - 1;
          }  
          
          wakeHour = tempset;
        }
      break;

      case 4: //Alarm minute adjust
        tempset = wakeMinute;
        
        if (newPos != position){    //if the rotary encoder is turned..
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          if (newPos > position){
            tempset = (tempset + 1) % 60;
          } else {
            tempset = (tempset == 0) ? 59: tempset - 1;
          }  
          
          wakeMinute = tempset;
        }
      break;

      case 5: //Adjust alarm song
        if (newPos != position){
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          delay(100);

          if(newPos > position){
            songIndex = (songIndex + 1) % 7;
          } else {
            songIndex = (songIndex == 0) ? 6: songIndex - 1;
          }
        }
      break;

      case 6: //Adjust alarm volume
        if (newPos != position){
          //reset dim and mode timers
          framecount = 0;           
          dimcount = 0;
          if(newPos > position){
            songVolume = (songVolume + 1) % 31;
          } else {
            songVolume = (songVolume == 0) ? 30: songVolume -1;
          }
        }
      break;
      } //end of switch mode block
  
    prev = now;
    position = newPos;

  }//end of update if-statement

  if(!digitalRead(setButton)){
    delay(25);                          //debounce

    if(!digitalRead(setButton)){        //if set button is pressed:
      mode = (mode + 1) % 7;            //increase the mode by one, and rollover at 4
      framecount = 0;                   //reset frame counts
      dimcount = 0;                     //reset dimming counter
      while(!digitalRead(setButton)){}  //wait for button release
    }
  }

  alarmSet = digitalRead(alarmButton);

}//end of loop

//function to set the time of the RTC to the time the code was compiled
void setCompileTime(){
  // Define arrays for month conversion and use sscanf for parsing __DATE__ and __TIME__
  const char monthNames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char monthStr[4];
  int year, month, day, hour, minute, second;
  
  // Parse compile date and time
  sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);
  sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);
  
  // Convert month string (first 3 letters of __DATE__) to a number
  month = (strstr(monthNames, monthStr) - monthNames) / 3 + 1;

  // Set the RTC time
  rtc.setTime(year, month, day, hour, minute, second);
}

void getAlarm(){
  //sTimeData_t t, alarmStart, alarmTime, alarmStop;
  alarmTime.day = t.day;
  alarmTime.hour = wakeHour;
  alarmTime.minute = wakeMinute;

  //convert current time into Unix time
  convertTimetoUnix = t.day*216000 + t.hour*3600 + t.minute*60 + t.second;
  //convert alarmtime into Unix time
  convertATtoUnix = alarmTime.day*216000 + alarmTime.hour*3600 + alarmTime.minute*60 + alarmTime.second;
  //subtract 20 minutes from Unix time for alarm start time
  alarmStartUnix = convertATtoUnix - 21*60;
  //add 5 minutes to Unix time for alarm stopping time
  alarmStopUnix = convertATtoUnix + 5*60;

  //compare current time t to alarmstart, alarmtime and alarmstop
  //set alarmCase depending on situation
  //case 0: before AlarmStart, or after AlarmStop
  //case 1: inside of AlarmStart, but before AlarmTime - ramp-up phase
  //case 2: inside AlarmTime, but before AlarmStop - alarm sounding time
  if(convertTimetoUnix < alarmStartUnix){
    alarmCase = 0;
  } else if(convertTimetoUnix >= alarmStartUnix && convertTimetoUnix < convertATtoUnix){
    alarmCase = 1;
  } else if(convertTimetoUnix >= convertATtoUnix && convertTimetoUnix < alarmStopUnix){
    alarmCase = 2;
  } else if(convertTimetoUnix > alarmStopUnix){
    alarmCase = 0;
  } else {
    alarmCase = 0;
  }
}

void updateLights(){
  //check if reading time is set > set to reading brightness
  if(readingTime > 0){
    if(millis() >= dimFlag){
      readingTime = readingTime - 1;
      if(readingTime < 0){
        readingTime = 0;
      }
      dimFlag = millis() + 3600000;
    }
    
    secToRead = max(0, (readEnd - convertTimetoUnix));

    if(readingTime > 10){
      brightnessRL = 255;
    } else if (readingTime == 0){
      brightnessRL = 0;
    } else {
      brightnessRL = 0.425*secToRead; //0.425
    }
  } else {
    brightnessRL = 0;
  }
  
  if(alarmSet){
    switch (alarmCase){
      case 0: //before alarmStart or after alarmStop
        brightnessLED = 0;
      break;
      case 1: //after alarmStart, before alarmTime
        //calculate how many seconds until alarmTime
        unsigned long secsToAlarm = convertATtoUnix - convertTimetoUnix;
        //255 brightness steps / 1200 seconds to alarm = 0.21 brightness/second
        //inverted (255-brightness) to go from off to on
        //max function to prevent below zero calculation of the brightness
        brightnessBlue = max(0, 255 - (secsToAlarm * 0.21));
        //there is an offset between the LEDs; blue first, then red, then orange
        //brightness red = brightness blue - 5min
        //brightness orange = brightness blue - 12min?
        brightnessPurple = max(0, brightnessBlue - (0.21*300));
        brightnessOrange = max(0, brightnessBlue - (0.21*600));
      break;
      case 2: //during alarm
        brightnessLED = 255;
      break;
    }
  } else {
    brightnessLED = 0;
    brightnessBlue = 0;
    brightnessPurple = 0;
    brightnessOrange = 0;
  }
  
  int y = max(brightnessLED, brightnessRL);
  analogWrite(blueLED, max(brightnessBlue, y));
  analogWrite(purpleLED, max(brightnessPurple, y));
  analogWrite(orangeLED, max(brightnessOrange, y));
}

void updateSound(){
  audioController.setVolume(songVolume);
  
  if(alarmSet){
    switch (alarmCase){
      case 0:
        audioController.stop();
      break;
      case 2:
        if(!digitalRead(SFX_BUSY)){
          audioController.playFileIndex(songIndex + 1);
        }
      break;
    }
  }
  if(!alarmSet){
    audioController.stop();
  }
}

void updateDisplay(){
  //if enough time has passed; dim the display
  if(dimcount > 30 && dimming && alarmCase != 2){
    tft.fillRect(0, 0, 240, 240, GC9A01A_BLACK);
    return;
  }
  
  //Draw readingTime
  if(readingTime >= 1){
    char readT[4];
    sprintf(readT, "%2d", readingTime);
    if(mode >= 1){
      tft.setTextColor(ColorTwo, GC9A01A_BLACK);
    } else {tft.setTextColor(ColorOne, GC9A01A_BLACK);}
    tft.setCursor(oneCursor_x, oneCursor_y);
    tft.setTextSize(3);
    tft.print(readT);
  } else {
    tft.fillRect(oneCursor_x, oneCursor_y, 136, 21, GC9A01A_BLACK);
  }

  //Draw Time
  if(mode <= 4){
    //convert current time to strings to display
    sprintf(timeHour, "%2hhu", t.hour);
    sprintf(timeMin, "%02hhu", t.minute);
    
    tft.setTextSize(6);
    
    //Draw time "hh"
    tft.setCursor(twoCursor_x, twoCursor_y);
    if(mode >= 2){
      tft.setTextColor(ColorTwo, GC9A01A_BLACK);
    } else {tft.setTextColor(ColorOne, GC9A01A_BLACK);}
    tft.print(timeHour);

    //Draw time ":"
    tft.setCursor(threeCursor_x, threeCursor_y);
    if(mode >= 1){
      tft.setTextColor(ColorTwo, GC9A01A_BLACK);
    } else {tft.setTextColor(ColorOne, GC9A01A_BLACK);}
    tft.print(":");
    
    //Draw time "mm"
    tft.setCursor(fourCursor_x, fourCursor_y);
    if(mode == 0 || mode == 2){
      tft.setTextColor(ColorOne, GC9A01A_BLACK);
    } else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
    tft.print(timeMin);
  }

  //Draw song titles and volume
  if(mode >= 5){
    if(framecount < 2){tft.fillRect(0, 0, 240, 240, GC9A01A_BLACK);}
    if(mode == 5){tft.setTextColor(ColorOne, GC9A01A_BLACK);}
    else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
    tft.setCursor(fiveCursor_x, fiveCursor_y);
    tft.setTextSize(4);
    tft.print(songs[songIndex]);

    //Draw volume setting"
    tft.setCursor(sevenCursor_x, sevenCursor_y);
    if(mode == 6){tft.setTextColor(ColorOne, GC9A01A_BLACK);}
    else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
    tft.setTextSize(3);
    tft.print(songVolume);
  }

  //Draw alarm icon and time
  if(mode <= 4){
    //if the alarm is ON, display alarm time
    if(alarmSet){
      sprintf(alarmTimeHour, "%2hhu", wakeHour);
      sprintf(alarmTimeMin, "%02hhu", wakeMinute);

      tft.setTextSize(3);

      //draw icon
      if(mode >= 1){tft.drawBitmap(sixCursor_x, sixCursor_y, sunIcon, 40, 40, ColorTwo);}
      else {tft.drawBitmap(sixCursor_x, sixCursor_y, sunIcon, 40, 40, ColorOne);}
      
      //Draw alarm "hh"
      tft.setCursor(sevenCursor_x, sevenCursor_y);
      if(mode == 0 || mode == 3){
      tft.setTextColor(ColorOne, GC9A01A_BLACK);
      } else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
      tft.print(alarmTimeHour);

      //Draw alarm ":"
      tft.setCursor(eightCursor_x, eightCursor_y);
      if(mode == 0){
      tft.setTextColor(ColorOne, GC9A01A_BLACK);
      } else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
      tft.print(":");
    
      //Draw alarm "mm"
      tft.setCursor(nineCursor_x, nineCursor_y);
      if(mode == 0 || mode == 4){
      tft.setTextColor(ColorOne, GC9A01A_BLACK);
      } else {tft.setTextColor(ColorTwo, GC9A01A_BLACK);}
      tft.print(alarmTimeMin);
      }

      //if no alarm is set: fill area with black
      else{ tft.fillRect(sixCursor_x, sixCursor_y, 143, 42, GC9A01A_BLACK);}
  }
}