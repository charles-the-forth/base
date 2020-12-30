#include <SD.h>
#include <SDCard.h>
#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

#define Serial SerialUSB

uint8_t screenNum = 1;
uint32_t messageId;

const int SCREENS_COUNT = 2;
const int PC_BAUDRATE = 57600;

const int BUTTON_1_PIN = 5;
const int BUTTON_2_PIN = 4;
const int BUTTON_3_PIN = 3;

#define NETWORKID       0
#define MYNODEID        2
#define FREQUENCY       RF69_433MHZ
#define FREQUENCY_SPECIFIC_HZ 443000000
#define chip_select_pin   43
#define interupt_pin    9
#define sd_cs_pin 35

RFM69 radio(chip_select_pin, interupt_pin, true);

Ucglib_ST7735_18x128x160_HWSPI ucg(6, 7, -1);
OcsGraphics ocsDesign(ucg);
OcsStorage ocsData(ocsDesign);
OcsStorage::message1 data1;
OcsStorage::message2 data2;
OcsStorage::message3 data3;
OcsStorage::message4 data4;

File file;
SDCard sdCard;

String csvFilename = sdCard.getFilename();
bool isSDCardInitialised = false;
bool isRadioOk = true;
bool debugLog = true;

void setup()
{
  Serial.begin(PC_BAUDRATE);

  ucg.begin(UCG_FONT_MODE_TRANSPARENT);

  ucg.clearScreen();

  ocsDesign.drawHomescreen();

  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  pinMode(BUTTON_3_PIN, INPUT);
  
  initRadio();
  initSDCard();

  if (isSDCardInitialised) {
    writeFileHeader();
  }
}

void loop() {
  int button1 = digitalRead(BUTTON_1_PIN);
  int button2 = digitalRead(BUTTON_2_PIN);
  int button3 = digitalRead(BUTTON_3_PIN);

  if(button1 == LOW) {
    if (screenNum > 1) {
      screenNum--;
      ocsDesign.drawScreen(screenNum);
      delay(300);
    }
  } else if (button2 == LOW && screenNum != 2) {
    screenNum = 1;
    ocsDesign.drawScreen(screenNum);
    delay(300);
  } else if (button3 == LOW) {
    if (screenNum < SCREENS_COUNT) {
      screenNum++;
      ocsDesign.drawScreen(screenNum);
      delay(300);
    }
  }

  if (radio.receiveDone()) {
    if (radio.DATA[0] == 1) {
      data1 = *(OcsStorage::message1*)radio.DATA;
      messageId = data1.messageId;
      ocsData.Update(data1, screenNum);

      if (debugLog) {       
        Serial.println("Message 1 received!"); 
      }
    } else if (radio.DATA[0] == 2) {
      data2 = *(OcsStorage::message2*)radio.DATA;
      messageId = data2.messageId;
      ocsData.Update(data2, screenNum);

      if (debugLog) {       
        Serial.println("Message 2 received!"); 
      }
    } else if (radio.DATA[0] == 3) {      
      data3 = *(OcsStorage::message3*)radio.DATA;
      messageId = data3.messageId;
      ocsData.Update(data3, screenNum);

      if (debugLog) {       
        Serial.println("Message 3 received!"); 
      }
    } else if (radio.DATA[0] == 4) {
      data4 = *(OcsStorage::message4*)radio.DATA;
      messageId = data4.messageId;
      ocsData.Update(data4, screenNum);

      if (debugLog) {       
        Serial.println("Message 4 received!"); 
      }
    }

    file = SD.open(csvFilename, FILE_WRITE);

    int rssi = radio.readRSSI();

    if (debugLog) {
      printAllReceivedData();
    }

    if (file) {
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
      file.println(String(data4.a) + ";" + String(data4.b) + ";" + String(data4.c) + ";" + String(data4.d) + ";" + String(data4.e) + ";" + String(data4.f) + ";" + String(data4.g) + ";" + String(data4.h) + ";" + String(data4.i) + ";" + String(data4.j) + ";" + String(data4.r) + ";" + String(data4.s) + ";" + String(data4.t));
      file.close();
    }
    
    if (debugLog) {
      Serial.println("-----------------------------------------------");  
    }    
  }
  delay(50);
}

void initRadio() {
  bool initialised = radio.initialize(FREQUENCY, MYNODEID, NETWORKID);

  if (initialised) {
    radio.setFrequency(FREQUENCY_SPECIFIC_HZ);
    radio.setHighPower(true);

    if (debugLog) {
      Serial.println("RFM69HW initialisation successful."); 
    }
  } else {
    isRadioOk = false;

    if (debugLog) {
      Serial.println("RFM69HW initialisation failed!"); 
    }
  }
}

void initSDCard() {
  isSDCardInitialised = SD.begin(sd_cs_pin);

  if (isSDCardInitialised && debugLog) {
    Serial.println("SD card initialisation successful.");
  } else if (debugLog) {   
    Serial.println("SD card initialisation failed!"); 
  }
}

void writeFileHeader() {
  file = SD.open(csvFilename, FILE_WRITE);

  if (file) {
      file.print("message;light;uvIndex;tempCanSat;tempMPU;tempExternal;tempSCD30;ambientTemp;objectTemp;humCanSat;humExternal;humSCD30;pressCanSat;pressExternal;altCanSat;");
      file.println("altExternal;accX;accY;accZ;rotX;rotY;rotZ;magX;magY;magZ;latInt;lonInt;latAfterDot;lonAfterDot;co2SCD30;co2CCS811;tvoc;o2Con;a;b;c;d;e;f;g;h;i;j;k;l;r;s;t;u;v;w;numberOfSatellites;RSSI;");
      file.close();
  }
}

void printAllReceivedData() {
  Serial.println("Message id: " + String(messageId));
  
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
}
