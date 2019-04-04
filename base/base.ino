#include <OcsStorage.h>
#include <RFM69.h>
#include <Ucglib.h>

#ifndef UCG_INTERRUPT_SAFE
#define UCG_INTERRUPT_SAFE
#endif

uint8_t screenNum = 3;

#define SCREENS_COUNT 6

#define Serial SerialUSB

// LOCAL
#define PC_BAUDRATE       57600

// BUTTONS
#define BUTTON_1 5
#define BUTTON_2 4
#define BUTTON_3 3

Ucglib_ST7735_18x128x160_HWSPI ucg(6, 7, -1);
OcsGraphics ocsDesign(ucg);


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
    }
    delay(100);
  } else if (button2 == LOW) {
    screenNum = 3;
    ocsDesign.drawScreen(screenNum);
    delay(100);
  } else if (button3 == LOW) {
    if (screenNum != SCREENS_COUNT) {
      screenNum++;
      ocsDesign.drawScreen(screenNum);
      delay(100);
    }
  }
  Serial.println("Screen: " + String(screenNum));
}
