#include "SDCard.h"
#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

uint8_t screenNum = 1;

#define SCREENS_COUNT 2

#define Serial SerialUSB

// LOCAL
#define PC_BAUDRATE       57600

// BUTTONS
#define BUTTON_1 5
#define BUTTON_2 4
#define BUTTON_3 3

// RFM69
#define NETWORKID       0   // Must be the same for all nodes (0 to 255)
#define MYNODEID        2   // My node ID (0 to 255)
#define FREQUENCY       RF69_433MHZ
#define FREQUENCYSPECIFIC 443000000  // Should be value in Hz, now 433 Mhz will be set
#define chip_select_pin   43
#define interupt_pin    9
#define sd_cs_pin 35

bool isRadioOk = true;

RFM69 radio(chip_select_pin, interupt_pin, true);

Ucglib_ST7735_18x128x160_HWSPI ucg(6, 7, -1);
OcsGraphics ocsDesign(ucg);
OcsStorage ocsData(ocsDesign);
SDCard sdCard;
OcsStorage::message income;
File file;
String csvFilename;

void setup()
{
  Serial.begin(PC_BAUDRATE);

  // Space behind font is transparent
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);

  // Clear the screen and reset the clip range to maximum
  ucg.clearScreen();

  ocsDesign.drawHomescreen();

  // Buttons
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  
  if(!radio.initialize(FREQUENCY, MYNODEID, NETWORKID))
  {
    isRadioOk = false;
    Serial.println("RFM69HW initialization failed!");
  }
  else
  {
    radio.setFrequency(FREQUENCYSPECIFIC);
    radio.setHighPower(true); // Always use this for RFM69HW
  }

  SD.begin(sd_cs_pin);

  csvFilename = sdCard.getFilename();
  file = SD.open(csvFilename, FILE_WRITE);

  if (file) {
    file.println("message;light;tempCanSat;tempExternal;ambientTemp;objectTemp;humCanSat;humExternal;pressCanSat;pressExternal;altCanSat;altExternal;numOfSats;latInt;lonInt;latAfterDot;lonAfterDot;co2SCD30;co2CCS811;tvoc;o2Con;RSSI");
    file.close();
  }
}

void loop()
{
  int button1 = digitalRead(BUTTON_1);
  int button2 = digitalRead(BUTTON_2);
  int button3 = digitalRead(BUTTON_3);

  if(button1 == LOW)
  {
    if (screenNum != 1) {
      screenNum--;
      ocsDesign.drawScreen(screenNum);
      delay(300);
    }
  } else if (button2 == LOW && screenNum != 2) {
    screenNum = 1;
    ocsDesign.drawScreen(screenNum);
    delay(300);
  } else if (button3 == LOW) {
    if (screenNum != SCREENS_COUNT) {
      screenNum++;
      ocsDesign.drawScreen(screenNum);
      delay(300);
    }
  }

  if (radio.receiveDone()) // Got one!
  {    
    income = *(OcsStorage::message*)radio.DATA;
    file = SD.open(csvFilename, FILE_WRITE);

    float rssi = radio.readRSSI();
    Serial.print("START;" + String(income.messageId) + ";" + String(income.lightIntensity) + ";" + String(income.uvIndex) + ";");
    Serial.print(String(income.temperatureCanSat) + ";" + String(income.temperatureMPU) + ";" + String(income.temperatureExternal) + ";");
    Serial.print(String(income.temperatureSCD30) + ";" + String(income.ambientTemp) + ";" + String(income.objectTemp) + ";");
    Serial.print(String(income.humidityCanSat) + ";"+ String(income.humidityExternal) + ";" + String(income.humiditySCD30) + ";");
    Serial.print(String(income.pressureCanSat) + ";" + String(income.pressureExternal) + ";" + String(income.altitudeCanSat) + ";");
    Serial.print(String(income.altitudeExternal) + ";" + String(income.accelerationX)+ ";" + String(income.accelerationY) + ";");
    Serial.print(String(income.accelerationZ) + ";" + String(income.rotationX) + ";" + String(income.rotationY) + ";");
    Serial.print(String(income.rotationZ) + ";" + String(income.magnetometerX) + ";" + String(income.magnetometerY) + ";");
    Serial.print(String(income.magnetometerZ) + ";" + String(income.latInt) + ";" + String(income.lonInt) + ";");
    Serial.print(String(income.latAfterDot) + ";" + String(income.lonAfterDot) + ";" + String(income.co2SCD30) + ";"  + String(income.co2CCS811) + ";");
    Serial.print(String(income.tvoc) + ";"  + String(income.o2Concentration) + ";" + String(income.a) + ";");
    Serial.print(String(income.b) + ";" + String(income.c) + ";" + String(income.d) + ";");
    Serial.print(String(income.e) + ";" + String(income.f) + ";" + String(income.g) + ";");
    Serial.print(String(income.h) + ";" + String(income.k) + ";" + String(income.l) + ";");
    Serial.print(String(income.r) + ";" + String(income.s) + ";" + String(income.t) + ";");
    Serial.println(String(income.u) + ";" + String(income.v) + ";" + String(income.w) + ";END");

    if (file)
    {
      file.print(String(income.messageId) + ";" + String(income.lightIntensity) + ";" + String(income.uvIndex) + ";");
      file.print(String(income.temperatureCanSat) + ";" + String(income.temperatureMPU) + ";" + String(income.temperatureExternal) + ";");
      file.print(String(income.temperatureSCD30) + ";" + String(income.ambientTemp) + ";" + String(income.objectTemp) + ";");
      file.print(String(income.humidityCanSat) + ";"+ String(income.humidityExternal) + ";" + String(income.humiditySCD30) + ";");
      file.print(String(income.pressureCanSat) + ";" + String(income.pressureExternal) + ";" + String(income.altitudeCanSat) + ";");
      file.print(String(income.altitudeExternal) + ";" + String(income.accelerationX)+ ";" + String(income.accelerationY) + ";");
      file.print(String(income.accelerationZ) + ";" + String(income.rotationX) + ";" + String(income.rotationY) + ";");
      file.print(String(income.rotationZ) + ";" + String(income.magnetometerX) + ";" + String(income.magnetometerY) + ";");
      file.print(String(income.magnetometerZ) + ";" + String(income.latInt) + ";" + String(income.lonInt) + ";");
      file.print(String(income.latAfterDot) + ";" + String(income.lonAfterDot) + ";" + String(income.co2SCD30) + ";"  + String(income.co2CCS811) + ";");
      file.print(String(income.tvoc) + ";"  + String(income.o2Concentration) + ";" + String(income.a) + ";");
      file.print(String(income.b) + ";" + String(income.c) + ";" + String(income.d) + ";");
      file.print(String(income.e) + ";" + String(income.f) + ";" + String(income.g) + ";");
      file.print(String(income.h) + ";" + String(income.k) + ";" + String(income.l) + ";");
      file.print(String(income.r) + ";" + String(income.s) + ";" + String(income.t) + ";");
      file.println(String(income.u) + ";" + String(income.v) + ";" + String(income.w));
      file.close();
    }
    ocsData.Update(income, screenNum);
  }
  delay(50);
}
