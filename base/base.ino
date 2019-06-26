#include <SD.h>
#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

uint8_t screenNum = 1;
uint32_t messageId;

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
OcsStorage::message1 data1;
OcsStorage::message2 data2;
OcsStorage::message3 data3;
OcsStorage::message4 data4;
File file;
String csvFilename = "result.csv";

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

  file = SD.open(csvFilename, FILE_WRITE);

  if (file) {
      file.print("message;light;uvIndex;tempCanSat;tempMPU;tempExternal;tempSCD30;ambientTemp;objectTemp;humCanSat;humExternal;humSCD30;pressCanSat;pressExternal;altCanSat;");
      file.println("altExternal;accX;accY;accZ;rotX;rotY;rotZ;magX;magY;magZ;latInt;lonInt;latAfterDot;lonAfterDot;co2SCD30;co2CCS811;tvoc;o2Con;a;b;c;d;e;f;g;h;i;j;k;l;r;s;t;u;v;w;numberOfSatellites;RSSI;");
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
    if (radio.DATA[0] == 1) {
      data1 = *(OcsStorage::message1*)radio.DATA;
      messageId = data1.messageId;

      //Serial.println("Message 1");  
    } else if (radio.DATA[0] == 2) {
      data2 = *(OcsStorage::message2*)radio.DATA;
      messageId = data2.messageId;

      //Serial.println("Message 2");  
    } else if (radio.DATA[0] == 3) {      
      data3 = *(OcsStorage::message3*)radio.DATA;
      messageId = data3.messageId;

      //Serial.println("Message 3");  
    } else if (radio.DATA[0] == 4) {
      data4 = *(OcsStorage::message4*)radio.DATA;
      messageId = data4.messageId;

      //Serial.println("Message 4");
    }
    file = SD.open(csvFilename, FILE_WRITE);

    int rssi = radio.readRSSI();

    Serial.print("START;" + String(messageId) + ";" + String(data1.lightIntensity) + ";" + String(data2.uvIndex) + ";");
    Serial.print(String(data1.temperatureCanSat) + ";" + String(data2.temperatureMPU) + ";" + String(data1.temperatureExternal) + ";");
    Serial.print(String(data2.temperatureSCD30) + ";" + String(data1.ambientTemp) + ";" + String(data1.objectTemp) + ";");
    Serial.print(String(data1.humidityCanSat) + ";"+ String(data1.humidityExternal) + ";" + String(data2.humiditySCD30) + ";");
    Serial.print(String(data1.pressureCanSat) + ";" + String(data1.pressureExternal) + ";" + String(data1.altitudeCanSat) + ";");
    Serial.print(String(data1.altitudeExternal) + ";" + String(data3.accelerationX)+ ";" + String(data3.accelerationY) + ";");
    Serial.print(String(data3.accelerationZ) + ";" + String(data3.rotationX) + ";" + String(data3.rotationY) + ";");
    Serial.print(String(data3.rotationZ) + ";" + String(data3.magnetometerX) + ";" + String(data3.magnetometerY) + ";");
    Serial.print(String(data3.magnetometerZ) + ";" + String(data2.latInt) + ";" + String(data2.lonInt) + ";");
    Serial.print(String(data2.latAfterDot) + ";" + String(data2.lonAfterDot) + ";" + String(data1.co2SCD30) + ";"  + String(data1.co2CCS811) + ";");
    Serial.print(String(data2.tvoc) + ";"  + String(data2.o2Concentration) + ";");
    Serial.print(String(data4.a) + ";" + String(data4.b) + ";" + String(data4.c) + ";" + String(data4.d) + ";" + String(data4.e) + ";" + String(data4.f) + ";" + String(data4.g) + ";" + String(data4.h) + ";" + String(data4.i) + ";" + String(data4.j) + ";" + String(data4.r) + ";" + String(data4.s) + ";" + String(data4.t) + ";");
    Serial.println(String(data2.numberOfSatellites) + ";"  + String(rssi) + ";END");

    if (file)
    {
      file.print(String(messageId) + ";" + String(data1.lightIntensity) + ";" + String(data2.uvIndex) + ";");
      file.print(String(data1.temperatureCanSat) + ";" + String(data2.temperatureMPU) + ";");
      file.print(String(data1.temperatureExternal) + ";" + String(data2.temperatureSCD30) + ";" + String(data1.ambientTemp) + ";" + String(data1.objectTemp) + ";" + String(data1.humidityCanSat) + ";"+ String(data1.humidityExternal) + ";" + String(data2.humiditySCD30) + ";");
      file.print(String(data1.pressureCanSat) + ";" + String(data1.pressureExternal) + ";");
      file.print(String(data1.altitudeCanSat) + ";" + String(data1.altitudeExternal) + ";" + String(data3.accelerationX)+ ";");
      file.print(String(data3.accelerationY) + ";" + String(data3.accelerationZ) + ";" + String(data3.rotationX) + ";");
      file.print(String(data3.rotationY) + ";" + String(data3.rotationZ) + ";" + String(data3.magnetometerX) + ";");
      file.print(String(data3.magnetometerY) + ";" + String(data3.magnetometerZ) + ";" + String(data2.numberOfSatellites) + ";");
      file.print(String(data2.latInt) + ";"  + String(data2.lonInt) + ";"  + String(data2.latAfterDot) + ";" + String(data2.lonAfterDot) + ";");
      file.print(String(data1.co2SCD30) + ";"  + String(data1.co2CCS811) + ";"  + String(data2.tvoc) + ";"  + String(data2.o2Concentration) + ";");
      file.print(String(data4.a) + ";" + String(data4.b) + ";" + String(data4.c) + ";" + String(data4.d) + ";" + String(data4.e) + ";" + String(data4.f) + ";" + String(data4.g) + ";" + String(data4.h) + ";" + String(data4.i) + ";" + String(data4.j) + ";" + String(data4.r) + ";" + String(data4.s) + ";" + String(data4.t) + ";");
      file.println(String(data2.numberOfSatellites) + ";"  + String(rssi));
      file.close();
    }
    /*Serial.println("Message id: " + String(messageId));
  
    Serial.println("Light intensity: " + String(data1.lightIntensity));
    
    Serial.println("UV sensor: " + String(data2.uvIndex));

    Serial.println("Temperature CanSat: " + String(data1.temperatureCanSat));
    Serial.println("Temperature MPU: " + String(data2.temperatureMPU));
    Serial.println("Temperature External: " + String(data1.temperatureExternal));
    Serial.println("Temperature SCD30: " + String(data2.temperatureSCD30));
    Serial.println("Ambient temperature: " + String(data1.ambientTemp));
    Serial.println("Object temperature: " + String(data1.objectTemp));
  
    Serial.println("Pressure CanSat: " + String(data1.pressureCanSat));
    Serial.println("Pressure External: " + String(data1.pressureExternal));

    Serial.println("Humidity CanSat: " + String(data1.humidityCanSat));
    Serial.println("Humidity External: " + String(data1.humidityExternal));
    Serial.println("Humidity SCD30: " + String(data2.humiditySCD30));

    Serial.println("Altitude CanSat: " + String(data1.altitudeCanSat));
    Serial.println("Altitude External: " + String(data1.altitudeExternal));

    Serial.println("O2: " + String(data2.o2Concentration) + " %");
    Serial.println("Acceleration X: " + String(data3.accelerationX));
    Serial.println("Acceleration Y: " + String(data3.accelerationY));
    Serial.println("Acceleration Z: " + String(data3.accelerationZ));

    Serial.println("Rotation X: " + String(data3.rotationX));
    Serial.println("Rotation Y: " + String(data3.rotationY));
    Serial.println("Rotation Z: " + String(data3.rotationZ));

    Serial.println("Magnetometer X: " + String(data3.magnetometerX));
    Serial.println("Magnetometer Y: " + String(data3.magnetometerY));
    Serial.println("Magnetometer Z: " + String(data3.magnetometerZ));

    Serial.println("CO2 SCD30: " + String(data1.co2SCD30) + " ppm");
    Serial.println("CO2 CCS811: " + String(data1.co2CCS811) + " ppm");
    Serial.println("TVOC CCS811: " + String(data2.tvoc) + " ppb");

    Serial.println("A: " + String(data4.a));
    Serial.println("B: " + String(data4.b));
    Serial.println("C: " + String(data4.c));
    Serial.println("D: " + String(data4.d));
    Serial.println("E: " + String(data4.e));
    Serial.println("F: " + String(data4.f));
    Serial.println("G: " + String(data4.g));
    Serial.println("H: " + String(data4.h));
    Serial.println("I: " + String(data4.i));
    Serial.println("J: " + String(data4.j));
    Serial.println("R: " + String(data4.r));
    Serial.println("S: " + String(data4.s));
    Serial.println("T: " + String(data4.t));
    
    //ocsData.Update(data, screenNum);
    Serial.println("-----------------------------------------");*/
  }
  delay(50);
}
