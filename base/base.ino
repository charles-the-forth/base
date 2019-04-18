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

bool isRadioOk = true;

RFM69 radio(chip_select_pin, interupt_pin, true);

Ucglib_ST7735_18x128x160_HWSPI ucg(6, 7, -1);
OcsGraphics ocsDesign(ucg);
OcsStorage ocsData(ocsDesign);

OcsStorage::message income;

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
    /*Serial.println("messageId: " + String(income.messageId));
    Serial.println("temperatureCanSat: " + String(income.temperatureCanSat));
    Serial.println("temperatureMPU: " + String(income.temperatureMPU));
    Serial.println("temperatureExternal: " + String(income.temperatureExternal));
    Serial.println("pressureCanSat: " + String(income.pressureCanSat));
    Serial.println("pressureExternal: " + String(income.pressureExternal));
    Serial.println("humidityCanSat: " + String(income.humidityCanSat));
    Serial.println("humidityExternal: " + String(income.humidityExternal));
    Serial.println("altitudeCanSat: " + String(income.altitudeCanSat));
    Serial.println("altitudeExternal: " + String(income.altitudeExternal));
    Serial.println("uvIndex: " + String(income.uvIndex));
    Serial.println("lightIntensity: " + String(income.lightIntensity));
    Serial.println("accelerationX: " + String(income.accelerationX));
    Serial.println("accelerationY: " + String(income.accelerationY));
    Serial.println("accelerationZ: " + String(income.accelerationZ));
    Serial.println("rotationX: " + String(income.rotationX));
    Serial.println("rotationY: " + String(income.rotationY));
    Serial.println("rotationZ: " + String(income.rotationZ));
    Serial.println("year: " + String(income.year));
    Serial.println("month: " + String(income.month));
    Serial.println("day: " + String(income.day));
    Serial.println("hour: " + String(income.hour));
    Serial.println("minute: " + String(income.minute));
    Serial.println("second: " + String(income.second));
    Serial.println("numberOfSatellites: " + String(income.numberOfSatellites));
    Serial.println("latInt: " + String(income.latInt));
    Serial.println("lonInt: " + String(income.lonInt));
    Serial.println("latAfterDot: " + String(income.latAfterDot));
    Serial.println("lonAfterDot: " + String(income.lonAfterDot));*/
    income = *(OcsStorage::message*)radio.DATA;
    ocsData.Update(income, screenNum);
    delay(300);
  }
}
