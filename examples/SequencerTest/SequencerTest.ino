#include "MCP3221.h"
#include "MCP4728.h"
#include "Adafruit_NeoTrellis.h"
#include "Noside_Sequencer.h"
#include <Wire.h>

#define CLK_PIN 6
#define RST_PIN 7
#define TRG1_PIN 2
#define TRG2_PIN 3
#define TRG3_PIN 4
#define TRG4_PIN 5

#define SHORT_PRESS_THRESHOLD 200
#define POT_THRESHOLD 10

static uint32_t CH1_COLOR = 0xFF0000;


bool lastClk = false;
int keyActive;
unsigned long timePressed;
bool potChange = false;
int lastPotVal = 0;

unsigned long simClkTime;

Noside_Sequencer sequencer;
MCP3221 mcp3221(0x4D);

MCP4728 mcp4728;

Adafruit_NeoTrellis trellis;

//define a callback for key presses
TrellisCallback blink(keyEvent evt) {
  // Check is the pad pressed?
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    //   Serial.print("Key Pressed: ");
    //   Serial.print(evt.bit.NUM);
    //   Serial.println()

    if (keyActive < 0) {
      lastPotVal = mcp3221.getVoltage();
      keyActive = evt.bit.NUM;
      timePressed = millis();
      Serial.print("Key ");
      Serial.print(keyActive);
      Serial.print(" Pressed at ");
      Serial.println(timePressed);
    }
    setDisplay();

    // trellis.pixels.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, trellis.pixels.numPixels(), 0, 255))); //on rising
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    // or is the pad released?
    // trellis.pixels.setPixelColor(evt.bit.NUM, 0); //off falling
    Serial.print("Key Released: ");
    Serial.println(evt.bit.NUM);

    if (evt.bit.NUM == keyActive) {
      if (millis() - timePressed < SHORT_PRESS_THRESHOLD) {
        Serial.print("Toggle step");
        int curCh = sequencer.getCurChannel();
        int step = 16 - evt.bit.NUM;
        Serial.println(step);
        bool toggleStep = !sequencer.getStepActive(curCh, step);
        sequencer.setStepActive(curCh, step, toggleStep);
      }
      keyActive = -1;
    } else if (evt.bit.NUM > 11) {
      if (!potChange) {
        Serial.print("Change Channel: ");
        sequencer.setChannel(16 - evt.bit.NUM);
        Serial.println(sequencer.getCurChannel());
      }
    }
    potChange = false;
    setDisplay();
  }

  // Turn on/off the neopixels!
  // trellis.pixels.show();

  return 0;
}


void setup() {
  // put your setup code here, to run once:
  // for (int i = 0; i < 16; i++) {
  keyActive = -1;
  timePressed = 0;
  // }
  Serial.begin(9600);
  Wire.begin();

  Serial.println("Noside Sequencer Initial Testing...");

  delay(10);
  setupMCP4728();

  Serial.print(F("MCP3221... "));
  Serial.println(mcp3221.ping() ? (F("Not Connected")) : (F("Connected!")));

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




  pinMode(CLK_PIN, INPUT);
  pinMode(RST_PIN, INPUT);
  pinMode(TRG1_PIN, OUTPUT);
  pinMode(TRG2_PIN, OUTPUT);
  pinMode(TRG3_PIN, OUTPUT);
  pinMode(TRG4_PIN, OUTPUT);

  // Serial.print("sequencer current step: ");
  // Serial.println(sequencer.getCurStep());


  // trellis.pixels.setPixelColor(15 - 3, 0, 0, 255);  //on rising
  // trellis.pixels.show();

  simClkTime = millis();
}

void loop() {

  if (keyActive >= 0) {
    // Serial.println("Key Active");
    if (millis() - timePressed >= SHORT_PRESS_THRESHOLD) {
      int curVoltage = mcp3221.getVoltage();
      if (abs(curVoltage - lastPotVal) > POT_THRESHOLD) {
        potChange = true;
        // Serial.println(lastPotVal);
        // Serial.println(mcp3221.getVoltage());
        // Serial.println("check pot change");
        sequencer.setStepCV(sequencer.getCurChannel(), 16 - keyActive, 4095 - mcp3221.getVoltage());
      }
    }
  }

  trellis.read();  // interrupt management does all the work! :)

  delay(20);  //the trellis has a resolution of around 60hz


  if (checkClock()) {
    // Serial.println("CLOCK PULSE");
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
