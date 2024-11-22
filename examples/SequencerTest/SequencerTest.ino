#include "MCP3221.h"
#include "MCP4728.h"
#include "Adafruit_NeoTrellis.h"
#include "Noside_Sequencer.h"
#include <Wire.h>

#define TRG1_PIN 2
#define TRG2_PIN 3
#define TRG3_PIN 4
#define TRG4_PIN 5
#define CLK_PIN 6
#define RST_PIN 7

#define SHORT_PRESS_THRESHOLD 200
#define POT_THRESHOLD 10

//todo add reset channel
//todo add multiple pages of steps per channel

const byte font[6][4] = {
  { 0xF, 0x2, 0x4, 0xF },  //N
  { 0xF, 0x9, 0x9, 0xF },  //O
  { 0xB, 0xB, 0xD, 0xD },  //S
  { 0x9, 0xF, 0xF, 0x9 },  //I
  { 0xF, 0x9, 0x9, 0x6 },  //D
  { 0xF, 0xB, 0x9, 0x9 }   //E
};

/*
N
1 0 0 1
1 1 0 1
1 0 1 1
1 0 0 1
F 2 4 F

O
1 1 1 1
1 0 0 1
1 0 0 1
1 1 1 1

S
1 1 1 1
1 1 0 0
0 0 1 1
1 1 1 1

I
1 1 1 1 
0 1 1 0
0 1 1 0
1 1 1 1

D
1 1 1 0
1 0 0 1
1 0 0 1
1 1 1 0

E
1 1 1 1
1 1 0 0
1 0 0 0
1 1 1 1

*/

bool lastClk = false;
int keyActive = -1;
unsigned long timePressed = 0;
bool potChange = false;
int lastPotVal = 0;

unsigned long simClkTime;

unsigned long flashChTime;

int CV_MAX = 4095;   //calibrated to +5V
int CV_MIN = 0;      //calibrated to -5V
int CV_ZERO = 2048;  //calibrated to 0V


Noside_Sequencer sequencer;
MCP3221 mcp3221(0x4D);
MCP4728 mcp4728;
Adafruit_NeoTrellis trellis;

//define a callback for key presses
TrellisCallback blink(keyEvent evt) {
  // Check for press event
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {

    //key active -1 when no other key pressed
    if (keyActive < 0) {
      lastPotVal = mcp3221.getVoltage();
      keyActive = evt.bit.NUM;
      timePressed = millis();

    } else if (keyActive == 3) {  //Change Channel
      if (potChange) {
        return;
      }
      int ch = 16 - evt.bit.NUM;
      if (ch > 4) {
        return;
      }
      sequencer.setChannel(ch);
      setDisplay();

      //flash button that is pressed
      flashButton(evt.bit.NUM, ch);


    } else if (keyActive == 0) {  // set octave range of current channel
      if (potChange) {
        return;
      }
      if (16 - evt.bit.NUM > 4) {
        return;
      }
      int curCh = sequencer.getCurChannel();
      sequencer.setCVRange(curCh, 16 - evt.bit.NUM);
      Serial.println(sequencer.getCVRange(curCh));
      flashButton(evt.bit.NUM, curCh);
      // int CVRange = sequencer.getCVRange(sequencer.getCurChannel());
    }

    // Check for release event
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    // Serial.print("Key Released: ");
    // Serial.println(evt.bit.NUM);

    if (evt.bit.NUM == keyActive) {
      if (millis() - timePressed < SHORT_PRESS_THRESHOLD) {
        // Serial.print("Toggle step");
        int curCh = sequencer.getCurChannel();
        int step = 16 - evt.bit.NUM;
        // Serial.println(step);
        bool toggleStep = !sequencer.getStepActive(curCh, step);
        sequencer.setStepActive(curCh, step, toggleStep);
      }
      keyActive = -1;
    }
    // else if (evt.bit.NUM > 11) {
    //   if (!potChange) {
    //     // Serial.print("Change Channel: ");
    //     sequencer.setChannel(16 - evt.bit.NUM);
    //     // Serial.println(sequencer.getCurChannel());
    //   }
    // }
    potChange = false;
  }
  setDisplay();

  return 0;
}


void setup() {

  pinMode(CLK_PIN, INPUT);
  pinMode(RST_PIN, INPUT);
  pinMode(TRG1_PIN, OUTPUT);
  pinMode(TRG2_PIN, OUTPUT);
  pinMode(TRG3_PIN, OUTPUT);
  pinMode(TRG4_PIN, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  Serial.println("Noside Sequencer...");

  delay(10);
  setupMCP4728();

  setupMCP3221();

  setupNeoTrellis();

  simClkTime = millis();

  clearScreen();
  fadeName();
  snakeWhiteBox();
}

void clearScreen() {
  for (int i = 0; i < 16; i++) {
    trellis.pixels.setPixelColor(i, 0, 0, 0);
  }
}

void snakeWhiteBox() {
  int brightness = 50;

  for (int i = 3; i >= 0; i--) {
    clearScreen();
    trellis.pixels.setPixelColor(i, brightness, brightness, brightness);
    trellis.pixels.show();
    delay(50);
  }
    for (int i = 4; i <= 7; i++) {
    clearScreen();
    trellis.pixels.setPixelColor(i, brightness, brightness, brightness);
    trellis.pixels.show();
    delay(50);
  }
    for (int i = 11; i >= 8; i--) {
    clearScreen();
    trellis.pixels.setPixelColor(i, brightness, brightness, brightness);
    trellis.pixels.show();
    delay(50);
  }
    for (int i = 12; i <= 14; i++) {
    clearScreen();
    trellis.pixels.setPixelColor(i, brightness, brightness, brightness);
    trellis.pixels.show();
    delay(50);
  }
}

void fadeName() {
  clearScreen();

  for (int i = 0; i < 6; i++) {

    byte col1 = font[i][0];
    byte col2 = font[i][1];
    byte col3 = font[i][2];
    byte col4 = font[i][3];

    for (int j = 50; j >= 0; j--) {
      displayText(col1, col2, col3, col4, j);
      delay(5);
    }
  }
}

void displayText(byte col1, byte col2, byte col3, byte col4, int brightness) {
  if (brightness > 50) {
    brightness = 50;
  } else if (brightness < 0) {
    brightness = 0;
  }

  for (int i = 0; i < 4; i++) {
    if (bitRead(col1, i)) {
      trellis.pixels.setPixelColor(15 - (4 * i), brightness, 0, 0);
    } else {
      trellis.pixels.setPixelColor(15 - (4 * i), 0, 0, 0);
    }

    if (bitRead(col2, i)) {
      trellis.pixels.setPixelColor(14 - (4 * i), brightness, 0, 0);
    } else {
      trellis.pixels.setPixelColor(14 - (4 * i), 0, 0, 0);
    }

    if (bitRead(col3, i)) {
      trellis.pixels.setPixelColor(13 - (4 * i), brightness, 0, 0);
    } else {
      trellis.pixels.setPixelColor(13 - (4 * i), 0, 0, 0);
    }

    if (bitRead(col4, i)) {
      trellis.pixels.setPixelColor(12 - (4 * i), brightness, 0, 0);
    } else {
      trellis.pixels.setPixelColor(12 - (4 * i), 0, 0, 0);
    }
  }
  trellis.pixels.show();
}

void loop() {
  trellis.read();  // interrupt management does all the work! :)

  if (keyActive >= 0) {
    // Serial.println("Key Active");
    if (millis() - timePressed >= SHORT_PRESS_THRESHOLD) {
      int curVoltage = mcp3221.getVoltage();
      if (abs(curVoltage - lastPotVal) > POT_THRESHOLD) {
        potChange = true;
        // Serial.println(lastPotVal);
        // Serial.println(mcp3221.getVoltage());
        // Serial.println("check pot change");

        int rawCV = 4095 - mcp3221.getVoltage();
        int curCh = sequencer.getCurChannel();
        int CVRange = sequencer.getCVRange(curCh);

        //todo set step CV to adjusted CV range and calibrated values.

        sequencer.setStepCV(curCh, 16 - keyActive, rawCV);
      }
    }
  }



  if (checkClock()) {
    sequencer.incrementStep();
    digitalWrite(TRG1_PIN, sequencer.getCurStepActive(4));
    digitalWrite(TRG2_PIN, sequencer.getCurStepActive(3));
    digitalWrite(TRG3_PIN, sequencer.getCurStepActive(2));
    digitalWrite(TRG4_PIN, sequencer.getCurStepActive(1));

    mcp4728.analogWrite(sequencer.getCurStepCV(4), sequencer.getCurStepCV(3),
                        sequencer.getCurStepCV(2), sequencer.getCurStepCV(1));

    setDisplay();
    // Serial.println(sequencer.getCurStep());
    // trellis.pixels.setPixelColor(sequencer.getCurStep(), 0, 0, 255); //on rising
  }
}

void setupMCP4728() {
  mcp4728.attatch(Wire, 14);
  delay(10);
  mcp4728.selectVref(MCP4728::VREF::VDD, MCP4728::VREF::VDD, MCP4728::VREF::VDD, MCP4728::VREF::VDD);
  mcp4728.selectGain(MCP4728::GAIN::X1, MCP4728::GAIN::X1, MCP4728::GAIN::X1, MCP4728::GAIN::X1);
  mcp4728.analogWrite(2048, 2048, 2048, 2048);
  mcp4728.enable(true);
  delay(10);
  mcp4728.readRegisters();

  // delay(1000);
  // for (int i = 0; i < 4; i++) {
  //   Serial.println(mcp4728.getDACData(i));
  // }
  Serial.print("MCP4728... ");
  if (mcp4728.getDACData(0) == 2048) {
    Serial.println("Connected!");
  } else {
    Serial.println("Not Connected");
  }
  // delay(100);
}

void setupMCP3221() {
  Serial.print(F("MCP3221... "));
  Serial.println(mcp3221.ping() ? (F("Not Connected")) : (F("Connected!")));
}

void setupNeoTrellis() {
  Serial.print("NeoTrellis... ");
  if (!trellis.begin()) {
    Serial.println("Not Connected");
    while (1) delay(1);
  } else {
    Serial.println("Connected!");
  }
  //activate all keys and set callbacks
  for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
  }
}

bool checkClock() {
  bool clockPulse = false;
  // bool clkIn = digitalRead(CLK_PIN);
  bool clkIn = false;
  // Serial.println(clkIn);

  unsigned long curTime = millis();
  if (curTime - simClkTime > 1000) {
    clkIn = true;
    simClkTime = curTime;
  }

  if (clkIn & !lastClk) {
    clockPulse = true;
    Serial.println("CLOCK PULSE");
  }

  lastClk = clkIn;
  return clockPulse;
}

bool checkReset() {
  bool resetActive = false;
  resetActive = digitalRead(RST_PIN);
  return resetActive;
}

void setDisplay() {
  int curCh = sequencer.getCurChannel();
  int curStep = sequencer.getCurStep();

  // Serial.print("Current channel: ");
  // Serial.println(curCh);
  for (int i = 1; i <= 16; i++) {
    bool stepActive = sequencer.getStepActive(curCh, i);

    if (stepActive) {
      setPixelColor(16 - i, curCh, i == curStep);
    } else {
      setPixelColor(16 - i, -1, i == curStep);
    }
  }
  trellis.pixels.show();
}

void setPixelColor(int ind, int ch, bool curStep) {
  int brightness = 30;
  int r = 0;
  int g = 0;
  int b = 0;
  switch (ch) {
    case 1:
      r = brightness;
      break;
    case 2:
      g = brightness;
      break;
    case 3:
      b = brightness;
      break;
    case 4:
      r = brightness;
      b = brightness;
      break;
    default:
      break;
  }
  if (curStep) {
    r += 50;
    g += 50;
    b += 50;
  }
  trellis.pixels.setPixelColor(ind, r, g, b);
}

int flashButton(int ind, int ch) {
  setPixelColor(ind, ch, false);
  trellis.pixels.show();
  delay(50);
  setPixelColor(ind, -1, false);
  trellis.pixels.show();
  delay(50);
  setPixelColor(ind, ch, false);
  trellis.pixels.show();
  delay(50);
  setPixelColor(ind, -1, false);
  trellis.pixels.show();
}
