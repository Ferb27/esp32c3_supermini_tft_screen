#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS   7
#define TFT_DC   2
#define TFT_RST  3
#define TFT_SCLK 4
#define TFT_MOSI 6

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 240);      // 1.54" 240x240
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(20, 40);
  tft.println("ESP32-C3 OK");

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(20, 80);
  tft.println("ST7789 OK");
}

void loop() {}