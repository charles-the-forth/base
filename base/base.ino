#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

uint8_t screenNum = 1;

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
#define FREQUENCYSPECIFIC 433000000  // Should be value in Hz, now 433 Mhz will be set
#define chip_select_pin   43
#define interupt_pin    9

bool isRadioOk = true;

RFM69 radio(chip_select_pin, interupt_pin, true);
// RFM69

Ucglib_ST7735_18x128x160_HWSPI ucg(6, 7, -1);
OcsGraphics ocsDesign(ucg);
OcsStorage ocsData(ocsDesign);

OcsStorage::message income;

void setup()
{
  Serial.begin(PC_BAUDRATE);

  //while(!Serial);

  Serial.println("PpenCanSat base station test started");

  // Space behind font is transparent
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);

  // Clear the screen and reset the clip range to maximum
  ucg.clearScreen();

  ocsDesign.drawBackground();
  ocsDesign.drawHomescreen();

  // Buttons
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);

  // RFM69
  Serial.print("OpenCansat ");
  Serial.print(MYNODEID,DEC);
  Serial.println(" is ready");

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
  //// RFM69
}

void loop()
{
  if (radio.receiveDone()) {
    income = *(OcsStorage::message*)radio.DATA;
    ocsData.Update(income, screenNum);
    Serial.println(String(income.temperatureCanSat) + ";" + String(income.temperatureExternal) + ";" + String(income.temperatureMPU) + ";" + String(income.pressureCanSat) + ";"
      + String(income.pressureExternal) + ";" + String(income.humidityCanSat) + ";" + String(income.humidityExternal) + ";" + String(income.accelerationX, 6) + ";" + String(income.accelerationY, 6) + ";"
      + String(income.accelerationZ, 6) + ";" + String(income.rotationX, 6) + ";" + String(income.rotationY, 6) + ";" + String(income.rotationZ, 6) + ";" + String(income.lightIntensity)
    );
    
    delay(100);
  }
}
