/************************************************************
 * CYD (ESP32-S3) HomeKit Switch with LCD UI using HomeSpan
 *
 * - Appears in Apple Home as a Switch accessory
 * - Displays current state + HomeKit setup code on the LCD
 * - Ready for you to add a QR bitmap for tap-to-pair
 *
 * Requirements:
 *   - ESP32 board support >= 3.1.0
 *   - HomeSpan library
 *   - TFT_eSPI library configured for the CYD
 ************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "HomeSpan.h"

// ---------- CONFIG ----------

// HomeKit pairing code: 8 digits, displayed as XXX-XX-XXX
// This MUST match what you set via HomeSpan setPairingCode()
static const char *PAIRING_CODE = "11112222";   // will show as 111-22-222
// Optional QR Setup ID (4 chars). Used only if you generate a real QR.
static const char *SETUP_ID     = "CYD1";

// ---------- GLOBALS ----------

TFT_eSPI tft;   // CYD TFT driver

// Forward declarations
void drawBaseScreen();
void drawSwitchState(bool on);
void drawHomeKitInfo();
void drawHomeKitQRPlaceholder();

// HomeSpan Service for a virtual switch that drives UI
struct CYDSwitch : Service::Switch {

  Characteristic::On *power;

  CYDSwitch() : Service::Switch() {
    power = new Characteristic::On(false);  // default OFF
  }

  // Called whenever HomeKit changes the switch
  boolean update() override {
    bool newVal = power->getNewVal();
    drawSwitchState(newVal);
    return true;
  }

  // Called to set the switch from local input (e.g. touch/button)
  void setFromLocal(bool state) {
    power->setVal(state);   // this notifies HomeKit as well
    drawSwitchState(state);
  }

  bool isOn() const {
    return power->getVal();
  }
};

CYDSwitch *hkSwitch = nullptr;

// Stub: replace this with your touch / button logic later
bool uiButtonPressed() {
  // TODO: read resistive touch, XPT2046, or a GPIO button.
  // Return true once per press.
  return false;
}

// ---------- HOMESPAN ACCESSORY SETUP ----------

void createAccessory() {

  new SpanAccessory();

    new Service::AccessoryInformation();
      new Characteristic::Name("CYD HomeKit Switch");
      new Characteristic::Manufacturer("DIY");
      new Characteristic::SerialNumber("CYD-SW-001");
      new Characteristic::Model("CYD-ESP32S3");
      new Characteristic::FirmwareRevision("1.0");
      new Characteristic::Identify();

    // Our HomeKit Switch service (single tile in Home app)
    hkSwitch = new CYDSwitch();
}

// ---------- LCD UI HELPERS ----------

void drawBaseScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);  // landscape on CYD; tweak if you prefer portrait

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.drawString("CYD HomeKit Switch", 10, 10);

  // Draw switch "button" outline in center
  int btnW = 180;
  int btnH = 80;
  int btnX = (tft.width()  - btnW) / 2;
  int btnY = (tft.height() - btnH) / 2;

  tft.drawRoundRect(btnX, btnY, btnW, btnH, 12, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Waiting...", tft.width()/2, tft.height()/2);

  drawHomeKitInfo();
}

void drawSwitchState(bool on) {

  // Re-draw the button area only
  int btnW = 180;
  int btnH = 80;
  int btnX = (tft.width()  - btnW) / 2;
  int btnY = (tft.height() - btnH) / 2;

  uint16_t fillColor = on ? TFT_GREEN : TFT_DARKGREY;
  uint16_t textColor = TFT_WHITE;

  tft.fillRoundRect(btnX+2, btnY+2, btnW-4, btnH-4, 10, fillColor);
  tft.drawRoundRect(btnX, btnY, btnW, btnH, 12, TFT_WHITE);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textColor, fillColor);
  tft.setTextSize(3);
  tft.drawString(on ? "ON" : "OFF", tft.width()/2, tft.height()/2);

  // Status text at the bottom
  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.fillRect(0, tft.height()-20, tft.width(), 20, TFT_BLACK);
  tft.drawString(on ? "HomeKit: Switch is ON" : "HomeKit: Switch is OFF",
                 tft.width()/2, tft.height()-2);
}

void drawHomeKitInfo() {
  // Show pairing code & QR placeholder along the bottom
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);

  // Pretty-print XXX-XX-XXX
  char formatted[12] = {0};
  snprintf(formatted, sizeof(formatted), "%.3s-%.2s-%.3s",
           PAIRING_CODE, PAIRING_CODE+3, PAIRING_CODE+5);

  int y = 40;
  tft.drawString("HomeKit Setup Code:", 10, y);
  tft.drawString(formatted,               10, y + 14);

  tft.drawString("Scan QR or enter code in the Home app", 10, y + 30);

  // Placeholder where your QR will be drawn
  drawHomeKitQRPlaceholder();
}

void drawHomeKitQRPlaceholder() {
  // Reserve a square in the upper-right for the QR code
  const int size = 90;
  int x = tft.width() - size - 10;
  int y = 50;

  tft.drawRect(x, y, size, size, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("QR HERE", x + size/2, y + size/2);

  // TODO: Once you have a QR bitmap, replace this function
  // with code that blits your 1-bit QR image into this square.
}

// ---------- ARDUINO SETUP / LOOP ----------

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Init LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // ---- HomeSpan config ----
  homeSpan.setLogLevel(1);               // less chatty logging
  homeSpan.setPairingCode(PAIRING_CODE); // 8 digits, no dashes
  homeSpan.setQRID(SETUP_ID);            // optional; used for QR payload

  // If you want Wi-Fi credentials baked in instead of using CLI:
  // homeSpan.setWifiCredentials("YourSSID", "YourPassword");

  homeSpan.begin(Category::Switches, "CYD Switch");

  createAccessory();

  drawBaseScreen();

  Serial.println();
  Serial.println("CYD HomeKit Switch starting...");
  Serial.println("Open Serial Monitor, type 'H' for HomeSpan help.");
}

void loop() {
  // HomeSpan core
  homeSpan.poll();

  // Local UI input: when you implement uiButtonPressed() this will
  // let local taps flip the HomeKit switch
  if (hkSwitch && uiButtonPressed()) {
    bool newState = !hkSwitch->isOn();
    hkSwitch->setFromLocal(newState);
  }

  // Add small delay to avoid hammering CPU if uiButtonPressed()
  // becomes more complex later.
  delay(10);
}
