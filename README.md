# CYD-Switch

## Overall architecture

Platform / tools

Board: ESP32-S3 “Cheap Yellow Display” (CYD)

IDE: Arduino IDE 2.x (or PlatformIO)

ESP32 core: ≥ 3.1.0 (needed for HomeSpan on S3) 
GitHub

HomeKit stack: HomeSpan (HomeKit implementation for ESP32 in Arduino) 
GitHub
+1

Display driver: TFT_eSPI configured for the CYD’s ILI9341/ST7789 panel 
Medium
+1

What the firmware will do

Boot, bring up Wi-Fi, start HomeSpan.

Expose a HomeKit Switch service (“CYD Switch”) that will show up as a normal on/off tile in the Home app.

Draw a simple UI on the CYD:

Title (“CYD HomeKit Switch”)

Big on/off state indicator

HomeKit setup code and a QR placeholder.

When paired for the first time, you’ll scan the QR / enter the code from the screen.

When:

The Home app toggles the switch, the LCD updates.

You later wire up touch or a physical button and flip it locally, HomeKit gets updated.

For QR: HomeSpan already calculates the X-HM://… payload for you and prints it to Serial when you set a pairing code; you can use that string with an external script (e.g. Achim Pieters’ esp32-homekit-qrcode) to generate a QR bitmap and embed it into flash, then draw it on the LCD. 
GitHub
+1

## Libraries you’ll need

Install via Arduino Library Manager:

HomeSpan

TFT_eSPI

Then configure TFT_eSPI for the CYD:

In the TFT_eSPI repo there are ready-made setups like Setup801_ESP32_CYD_ILI9341_240x320.h / Setup805_ESP32_CYD_ST7789_240x320.h. 
Medium
+1

Copy the correct one to User_Setup.h (or include it from there).

## Example sketch: CYD HomeKit Switch + LCD UI

This is a single-file Arduino sketch (cyd_hk_switch.ino) that:

Brings up HomeSpan,

Exposes a HomeKit Switch,

Draws a basic UI on the CYD,

Updates the display when the switch state changes.

QR drawing is left as a clearly marked TODO so you can plug in a pre-generated QR image or a QR library later.
