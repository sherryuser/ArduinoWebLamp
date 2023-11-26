/*
    Button/touch control
   - Hold - brightness
   - 1 tap - on/off
   - 2 taps - mode switch
   - 3 taps - on/off white mode
   - 4 taps - on/off autoplay
*/

// ************************** Settings for NOOBS ***********************
#define CURRENT_LIMIT 2000  // current limit in milliamps, automatically controls brightness (have mercy on your power supply!) 0 - turn off the limit
#define AUTOPLAY_TIME 30    // time between mode changes in seconds

#define NUM_LEDS 12         // number of LEDs in one piece of strip
#define NUM_STRIPS 4        // number of strips (in parallel)
#define LED_PIN 6           // strip pin
#define BTN_PIN 2           // touch pin
#define MIN_BRIGHTNESS 5    // minimum brightness for manual settings
#define BRIGHTNESS 250      // initial brightness
#define FIRE_PALETTE 0      // different types of fire (0 - 3). Try them all! =)

// ************************** Settings for PROs ***********************
#define MODES_AMOUNT 6

#include "GyverButton.h"
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

#include <FastLED.h>
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

#include "GyverTimer.h"
GTimer_ms effectTimer(60);
GTimer_ms autoplayTimer((long)AUTOPLAY_TIME * 1000);
GTimer_ms brightTimer(20);

int brightness = BRIGHTNESS;
int tempBrightness;
byte thisMode;

bool gReverseDirection = false;
boolean loadingFlag = true;
boolean autoplay = true;
boolean powerDirection = true;
boolean powerActive = false;
boolean powerState = true;
boolean whiteMode = false;
boolean brightDirection = true;
boolean wasStep = false;


// fill all
void fillAll(CRGB newcolor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = newcolor;
  }
}

// function for getting the color of a pixel by its number
uint32_t getPixColor(int thisPixel) {
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
  FastLED.setBrightness(brightness);
  FastLED.show();

  randomSeed(analogRead(0));
  touch.setTimeout(300);
  touch.setStepTimeout(50);

  if (FIRE_PALETTE == 0) gPal = HeatColors_p;
  else if (FIRE_PALETTE == 1) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  else if (FIRE_PALETTE == 2) gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  else if (FIRE_PALETTE == 3) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);
}

void loop() {
    if (Serial.available() > 0) {
        handleSerialInput();
    }

  touch.tick();
  if (touch.hasClicks()) {
    byte clicks = touch.getClicks();
    switch (clicks) {
      case 1:
        powerDirection = !powerDirection;
        powerActive = true;
        tempBrightness = brightness * !powerDirection;
        break;
      case 2: if (!whiteMode && !powerActive) {
          nextMode();
        }
        break;
      case 3: if (!powerActive) {
          whiteMode = !whiteMode;
          if (whiteMode) {
            effectTimer.stop();
            fillAll(CRGB::White);
            FastLED.show();
          } else {
            effectTimer.start();
          }
        }
        break;
      case 4: if (!whiteMode && !powerActive) autoplay = !autoplay;
        break;
      default:
        break;
    }
  }

  if (touch.isStep()) {
    if (!powerActive) {
      wasStep = true;
      if (brightDirection) {
        brightness += 5;
      } else {
        brightness -= 5;
      }
      brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
    }
  }

  if (touch.isRelease()) {
    if (wasStep) {
      wasStep = false;
      brightDirection = !brightDirection;
    }
  }

  if (effectTimer.isReady() && powerState) {
    switch (thisMode) {
      case 0: lighter();
        break;
      case 1: lightBugs();
        break;
      case 2: colors();
        break;
      case 3: rainbow();
        break;
      case 4: sparkles();
        break;
      case 5: fire();
        break;
      case 6: vinigret();
        break;
    }
    FastLED.show();
  }

  if (autoplayTimer.isReady() && autoplay) {    // mode change timer
    nextMode();
  }

  brightnessTick();
}

void nextMode() {
  thisMode++;
  if (thisMode >= MODES_AMOUNT) thisMode = 0;
  loadingFlag = true;
  FastLED.clear();
}

void brightnessTick() {
  if (powerActive) {
    if (brightTimer.isReady()) {
      if (powerDirection) {
        powerState = true;
        tempBrightness += 10;
        if (tempBrightness > brightness) {
          tempBrightness = brightness;
          powerActive = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      } else {
        tempBrightness -= 10;
        if (tempBrightness < 0) {
          tempBrightness = 0;
          powerActive = false;
          powerState = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      }
    }
  }
}

void handleSerialInput() {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("b")) {
        brightness = command.substring(1).toInt();
        brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
        FastLED.setBrightness(brightness);
        FastLED.show();
    } else if (command.startsWith("e")) {
        thisMode = command.substring(1).toInt();
        thisMode = constrain(thisMode, 0, MODES_AMOUNT - 1);
        loadingFlag = true;
        FastLED.clear();
    } else {
        handleSerialCommand(command.charAt(0));
    }
}

void handleSerialCommand(char command) {
    switch (command) {
        // Toggle power
        case 't':
            powerDirection = !powerDirection;
            powerActive = true;
            tempBrightness = brightness * !powerDirection;
            break;
        // Next effect
        case 'e': if (!whiteMode && !powerActive) {
          nextMode();
        } 
            break;
        // Increase brightness
        case 'b':
            changeBrightness(10); // Increase by 10, adjust as needed
            break;
        // Decrease brightness
        case 'd':
            changeBrightness(-10); // Decrease by 10, adjust as needed
            break;
        // Toggle autoplay   
        case 'a': if (!whiteMode && !powerActive) autoplay = !autoplay;
            break;
        // Toggle white mode
        case 'w': if (!powerActive) {
          whiteMode = !whiteMode;
          if (whiteMode) {
            effectTimer.stop();
            fillAll(CRGB::White);
            FastLED.show();
          } else {
            effectTimer.start();
          }
        }
        break;
    }
}

void changeBrightness(int change) {
    brightness = constrain(brightness + change, MIN_BRIGHTNESS, 255);
    FastLED.setBrightness(brightness);
    FastLED.show();
}

