/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

enum hvacState {
  Heating, //0
  Cooling, //1
  hCount //2
};

enum menuState {
  TemperatureMenu, //0
  OperationMenu, //1
  UnitMenu, //2
  mCount //3
};

enum tempState {
  C, //0
  F, //1
  TCount //2
};

hvacState opMode = Heating;
menuState menuMode = TemperatureMenu;
tempState tempMode = C;
float targetTemp = 26.;
float targetTempF = targetTemp * 9. / 5. + 32.;
volatile long prevChangeTime = 0;
volatile long prevChangeTimeTwo = 0;
long debounceTime = 50;
volatile bool changeButtonFlag = false;
volatile bool menuButtonFlag = false;

void IRAM_ATTR buttonToChangeThings() {
  long now = millis();
  if (now > prevChangeTime + debounceTime) {
    changeButtonFlag = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeMenu() {
  long now = millis();
  if (now > prevChangeTimeTwo + debounceTime) {
    menuButtonFlag = true;
    prevChangeTimeTwo = now;
  }
}

Adafruit_BME680 bme(&Wire); // I2C

float getCurrentTemp() {
  if(tempMode == tempState::C) {
    return bme.temperature;
  }
  if(tempMode == tempState::F) {
    return bme.temperature * 9. / 5. + 32.;
  }
  return -11111111.;
}


//Adafruit_BME680 bme(&Wire1); // example of I2C on another bus
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  pinMode(1, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(1), buttonToChangeThings, RISING);

  pinMode(2, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(2), buttonToChangeMenu, RISING);

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_2X);
  //bme.setHumidityOversampling(BME680_OS_2X);
  //bme.setPressureOversampling(BME680_OS_4X);
  //bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  //bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  float currentTemp = getCurrentTemp();
  Serial.print("Temperature = ");
  Serial.print(currentTemp);
  if(tempMode == tempState::C) {
    Serial.print(" *C");
  }
  if(tempMode == tempState::F) {
    Serial.print(" *F");
  }
  Serial.print(" with target ");
  if(tempMode == tempState::C) {
    Serial.print(targetTemp);
  }
  if(tempMode == tempState::F) {
    Serial.print(targetTempF);
  }
  Serial.print(" operating in mode ");
  if (opMode == 0) {
    Serial.print("Heating");
  }
  if (opMode == 1) {
    Serial.print("Cooling");
  }
  Serial.print(" in menu ");
  if (menuMode == 0) {
    Serial.println("Temperature Mode");
  }
  if (menuMode == 1) {
    Serial.println("Operation Mode");
  }
  if (menuMode == 2) {
    Serial.println("Unit Mode");
  }
  

  if (menuButtonFlag) {
    menuButtonFlag = false;
    menuMode = (menuState)(((int)menuMode + 1) % (int)menuState::mCount);
    Serial.print("!!!!!!!!!!!!!!!!!!!!!!! Moving to Menu: ");
    Serial.println(menuMode);
  }

  if (changeButtonFlag) {
    if (menuMode == TemperatureMenu) {
      targetTemp += 1.0;
      targetTempF += 1.0;
      if(tempMode == tempState::C) {
        if (targetTemp > 30.0) {
          targetTemp = targetTemp - 10.;
        }
      }
      if(tempMode == tempState::F) {
        if (targetTempF > 85.0) {
          targetTempF = targetTempF - 10.;
        }
      }
      
    }
  if (menuMode == OperationMenu) {
    opMode = (hvacState)(((int)opMode + 1) % (int)hvacState::hCount);
  }
  if (menuMode == UnitMenu) {
    tempMode = (tempState)(((int)tempMode + 1) % (int)tempState::TCount);
    // Change from F to C or C to F
  }
  changeButtonFlag = false;
  //opMode = (hvacState)(((int)opMode + 1) % (int)hvacState::hCount);
  }
  
  if(tempMode == tempState::C) {
    if (opMode == Heating) {
      if (currentTemp < targetTemp) {
        Serial.println("Heater is on now!");
      }
    } else if (opMode == Cooling) {
    if (currentTemp > targetTemp) {
      Serial.println("AC is on now!");
    }
    }
  }
  if(tempMode == tempState::F) {
    if (opMode == Heating) {
      if (currentTemp < targetTempF) {
        Serial.println("Heater is on now!");
      }
    } else if (opMode == Cooling) {
    if (currentTemp > targetTempF) {
      Serial.println("AC is on now!");
    }
    }
  }
   
  
/*
  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
*/
  Serial.println();
  delay(100);
}