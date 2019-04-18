#include "SDCard.h"
#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

uint8_t screenNum = 2;

#define SCREENS_COUNT 3

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
#define FREQUENCYSPECIFIC 433102000  // Should be value in Hz, now 433 Mhz will be set
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

  if (!SD.begin(sd_cs_pin)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");

  csvFilename = sdCard.getFilename();
  file = SD.open(csvFilename, FILE_WRITE);

  if (file) {
    file.println("message;light;tempCanSat;humCanSat;pressCanSat;altCanSat;year;month;day;hour;minute;second;numOfSats;latInt;lonInt;latAfterDot;lonAfterDot");
    file.close();
    Serial.println("File header written.");
  } else {
    Serial.println("Error writing file header.");
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
    screenNum = 2;
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
    file = SD.open(csvFilename, FILE_WRITE);
    if (file)
    {
      file.print(String(income.messageId) + ";" + String(income.temperatureCanSat) + ";" + String(income.pressureCanSat) + ";");
      file.print(String(income.humidityCanSat) + ";" + String(income.lightIntensity) + ";" + String(income.altitudeCanSat) + ";");
      file.print(String(income.year) + ";" + String(income.month) + ";" + String(income.day) + ";" + String(income.hour) + ";");
      file.print(String(income.minute) + ";" + String(income.second) + ";" + String(income.numberOfSatellites) + ";");
      file.println(String(income.latInt) + ";" + String(income.lonInt) + ";" + String(income.latAfterDot) + ";" + String(income.lonAfterDot));  
      file.close();
      Serial.println("Writing was successful.");
    } else {
      Serial.println("Error writing data.");
    }
    income = *(OcsStorage::message*)radio.DATA;
    ocsData.Update(income, screenNum);
    delay(300);
  }
}
