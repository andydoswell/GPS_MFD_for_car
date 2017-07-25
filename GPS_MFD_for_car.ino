/*
  GPS Multifunction display and head up display
  (C) Andy Doswell 23rd July 2017
  License: The MIT License (See full license at the bottom of this file)

  Schematic and decsription can be found at www.andydoz.blogspot.com/


  Uses the very excellent TinyGPS++ library from http://arduiniana.org/libraries/tinygpsplus/
  and the most superb I2C LCD library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
  I2C LCD library is here:https://playground.arduino.cc/Code/LCDi2c , however a modification is required
  to the library to enable it to work with the generic PCF8574 I2C LCD displays, see my website for more
  details.

*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
LiquidCrystal_I2C  lcd(0x3f, 16, 2); // I2C address 0x3f
TinyGPSPlus gps; //TinyGPS++ Serial
static   uint32_t GPSBaud = 57600; // GPS baudrate.
int hourTen; //tens of hours
int hourUnit; //units of hours
int minTen; // you get the idea..
int minUnit;
float MPH;
int loopCounter;
unsigned int hours; // whole hours displayed

byte antenna[8] = { // bitmap for antenna symbol
  0b00000,
  0b10101,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
};
byte fiveBarsLocked [8] = {
  0b10001,
  0b10001,
  0b11011,
  0b00011,
  0b00111,
  0b00111,
  0b01111,
  0b11111
};

byte  fourBarsLocked [8] = {
  0b10000,
  0b10000,
  0b11010,
  0b00010,
  0b00110,
  0b00110,
  0b01110,
  0b11110
};

byte threeBarsLocked[8] = {
  0b10000,
  0b10000,
  0b11000,
  0b00000,
  0b00100,
  0b00100,
  0b01100,
  0b11100
};

byte  twoBarsLocked[8] = {
  0b10000,
  0b10000,
  0b11000,
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b11000
};

byte oneBarLocked[8] = {
  0b10000,
  0b10000,
  0b11000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10000
};

byte  oneBarNoLock[8] = {
  0b10100,
  0b01000,
  0b10100,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10000
};

byte twoBarsNoLock[8] = {
  0b10100,
  0b01000,
  0b10100,
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b11000
};

byte threeBarsNoLock[8] = {
  0b10100,
  0b01000,
  0b10100,
  0b00000,
  0b00100,
  0b00100,
  0b01100,
  0b11100
};

byte fourBarsNoLock [8] = {
  0b10100,
  0b01000,
  0b10110,
  0b00010,
  0b00110,
  0b00110,
  0b01110,
  0b11110
};

byte fiveBarsNoLock[8] = {
  0b10101,
  0b01001,
  0b10111,
  0b00011,
  0b00111,
  0b00111,
  0b01111,
  0b11111
};
void setup() {
  lcd.init();
  lcd.begin(16, 2); // set up the LCD as 20 x 4 display
  //  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Waiting for GPS");
  delay (10000); // wait for other Arduino and GPS receiver to come up and configure GPS in runtime
  Serial.begin(GPSBaud); // start the comms with the GPS Rx
  lcd.clear();
  lcd.createChar (1, antenna);
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)1); // write antenna symbol
  lcd.createChar (0, oneBarNoLock);

}

boolean isBST() // this bit of code blatantly plagarised from http://my-small-projects.blogspot.com/2015/05/arduino-checking-for-british-summer-time.html
{
  int imonth = gps.date.month();
  int iday = gps.date.day();
  int hr = gps.time.hour();
  int rxYear = gps.date.year();
  //January, february, and november are out.
  if (imonth < 3 || imonth > 10) {
    return false;
  }
  //April to September are in
  if (imonth > 3 && imonth < 10) {
    return true;
  }

  // find last sun in mar and oct - quickest way I've found to do it
  // last sunday of march
  int lastMarSunday =  (31 - (5 * rxYear / 4 + 4) % 7);
  //last sunday of october
  int lastOctSunday = (31 - (5 * rxYear / 4 + 1) % 7);

  //In march, we are BST if is the last sunday in the month
  if (imonth == 3) {

    if ( iday > lastMarSunday)
      return true;
    if ( iday < lastMarSunday)
      return false;

    if (hr < 1)
      return false;

    return true;

  }
  //In October we must be before the last sunday to be bst.
  //That means the previous sunday must be before the 1st.
  if (imonth == 10) {

    if ( iday < lastOctSunday)
      return true;
    if ( iday > lastOctSunday)
      return false;

    if (hr >= 1)
      return false;

    return true;
  }

}

void displayData() {

  hours = (gps.time.hour());
  if (isBST()) {
    hours++;
  }
  if (hours == 24) {
    hours = 0;
  }
  hourUnit = (hours % 10);
  hourTen = ((hours / 10) % 10);
  minUnit = (gps.time.minute() % 10);
  minTen = ((gps.time.minute() / 10) % 10);

  lcd.setCursor(0, 0);
  lcd.write((uint8_t)1); // write antenna symbol
  lcd.setCursor(1, 0);
  lcd.write((uint8_t)0); // write signal strength
  lcd.print (" ");
  if (gps.speed.mph() > 3) {
    MPH = gps.speed.mph();
  }
  else {
    MPH = 0;
  }
  lcd.setCursor(3, 0);
  lcd.print(MPH, 1);
  lcd.print(" MPH ");
  lcd.print(TinyGPSPlus::cardinal(gps.course.value()));
  lcd.print (" ");
  lcd.setCursor(0, 1);
  lcd.print(gps.date.day());
  lcd.print("/");
  lcd.print(gps.date.month());
  lcd.print("/");
  lcd.print(gps.date.year());
  lcd.print("  ");
  lcd.print(hourTen);
  lcd.print(hourUnit);
  lcd.print(":");
  lcd.print(minTen);
  lcd.print (minUnit);

}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}

void updateSignal() {
  if (gps.location.isUpdated() && (gps.location.age() < 1000)) {
    if (gps.satellites.value() > 8) {
      lcd.createChar(0, fiveBarsLocked);
    }
    if (gps.satellites.value() < 8) {
      lcd.createChar(0, fourBarsLocked);
    }
    if (gps.satellites.value() < 6) {
      lcd.createChar(0, threeBarsLocked);
    }
    if (gps.satellites.value() < 5) {
      lcd.createChar(0, twoBarsLocked);
    }

    if (gps.satellites.value() < 4) {
      lcd.createChar(0, oneBarLocked);
    }
  }
  else {
    if (gps.location.age() > 1000) {
      if (gps.satellites.value() > 8) {
        lcd.createChar(0, fiveBarsNoLock);
      }
      if (gps.satellites.value() < 8) {
        lcd.createChar(0, fourBarsNoLock);
      }
      if (gps.satellites.value() < 6) {
        lcd.createChar(0, threeBarsNoLock);
      }
      if (gps.satellites.value() < 5) {
        lcd.createChar(0, twoBarsNoLock);
      }

      if (gps.satellites.value() < 4) {
        lcd.createChar(0, oneBarNoLock);
      }
    }
  }
}


void loop()
{

  loopCounter ++;
  while (Serial.available() > 0) //While GPS message received
    if (gps.encode(Serial.read()))

      if (loopCounter >= 100) {
        updateSignal();
        loopCounter = 0;
      }
  displayData();

}



/*
   Copyright (c) 2017 Andrew Doswell

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permiSerialion notice shall be included in
   all copies or substantial portions of the Software.

   Any commercial use is prohibited without prior arrangement.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESerial FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/
