#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS   7
#define TFT_DC   2
#define TFT_RST  3
#define TFT_SCLK 4
#define TFT_MOSI 6
#define BTN_PIN  5

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

unsigned long lastTick = 0;
int hh = 11, mm = 17, ss = 35;
int dayNum = 3, monthNum = 10, yearNum = 2025;
const char* weekDay = "Fri";
int tempNow = 30;
int humidity = 69;
const char* city = "Ha Nam";
const char* country = "VN";
const char* weatherLabel = "Clouds";
int face = 0;
bool lastBtn = HIGH;
unsigned long lastDebounce = 0;

uint16_t colBg = ST77XX_BLACK;
uint16_t colWhite = ST77XX_WHITE;
uint16_t colYellow = 0xFEE0;
uint16_t colCyan = 0x7FFF;
uint16_t colGreen = 0x07E0;
uint16_t colBadgeBlue = 0xC71F;
uint16_t colOrange = 0xFD20;

void drawCloud(int x, int y) {
  tft.fillCircle(x + 14, y + 16, 12, colWhite);
  tft.fillCircle(x + 30, y + 12, 14, colWhite);
  tft.fillCircle(x + 47, y + 18, 11, 0xF79E);
  tft.fillRoundRect(x + 8, y + 18, 44, 16, 8, colWhite);
}

void drawSunCloud(int x, int y) {
  tft.fillCircle(x + 18, y + 18, 10, colYellow);
  for (int i = 0; i < 8; i++) {
    float a = i * 0.785398;
    int x1 = x + 18 + cos(a) * 14;
    int y1 = y + 18 + sin(a) * 14;
    int x2 = x + 18 + cos(a) * 20;
    int y2 = y + 18 + sin(a) * 20;
    tft.drawLine(x1, y1, x2, y2, colYellow);
  }
  tft.fillCircle(x + 28, y + 26, 10, colYellow);
  tft.fillCircle(x + 42, y + 24, 12, colYellow);
  tft.fillRoundRect(x + 20, y + 28, 34, 12, 6, colYellow);
}

void drawThermo(int x, int y) {
  tft.drawCircle(x + 6, y + 17, 6, 0xFBE0);
  tft.fillCircle(x + 6, y + 17, 4, 0xFBE0);
  tft.drawRoundRect(x + 3, y, 6, 18, 3, 0xFBE0);
  tft.fillRect(x + 4, y + 5, 4, 9, 0xFBE0);
}

void drawDrop(int x, int y) {
  tft.fillTriangle(x + 6, y, x, y + 10, x + 12, y + 10, colCyan);
  tft.fillCircle(x + 6, y + 12, 6, colCyan);
}

void drawStaticFace0() {
  tft.fillScreen(colBg);

  tft.setTextWrap(false);
  tft.setTextSize(2);
  tft.setTextColor(colYellow, colBg);
  tft.setCursor(14, 10);
  tft.print(city);

  tft.fillRoundRect(152, 8, 46, 24, 6, colGreen);
  tft.setTextColor(colBg, colGreen);
  tft.setCursor(165, 14);
  tft.print(country);

  tft.setTextColor(colWhite, colBg);
  tft.setCursor(12, 40);
  tft.print("Max temp 30C");

  drawCloud(168, 34);

  tft.fillRoundRect(150, 92, 72, 22, 5, colWhite);
  tft.setTextColor(colBadgeBlue, colWhite);
  tft.setCursor(158, 98);
  tft.print(weatherLabel);

  tft.setTextColor(colWhite, colBg);
  tft.setTextSize(2);
  tft.setCursor(20, 158);
  tft.print(dayNum);
  tft.print("/");
  tft.print(monthNum);
  tft.print("/");
  tft.print(yearNum);

  tft.setTextColor(colGreen, colBg);
  tft.setCursor(168, 158);
  tft.print(weekDay);

  drawThermo(14, 194);
  drawDrop(15, 214);

  tft.drawRoundRect(58, 200, 78, 10, 4, colWhite);
  tft.fillRoundRect(60, 202, 48, 6, 3, colCyan);
  tft.drawRoundRect(58, 220, 78, 10, 4, colWhite);
  tft.fillRoundRect(60, 222, 54, 6, 3, colCyan);

  tft.setTextColor(colWhite, colBg);
  tft.setCursor(142, 194);
  tft.print(tempNow);
  tft.print("C");
  tft.setCursor(142, 214);
  tft.print(humidity);
  tft.print("%");

  drawSunCloud(172, 184);
}

void drawTimeBlock() {
  tft.fillRect(8, 64, 224, 84, colBg);

  tft.setTextSize(6);
  tft.setTextColor(colWhite, colBg);
  tft.setCursor(10, 78);
  if (hh < 10) tft.print("0");
  tft.print(hh);
  tft.print(":");

  tft.setTextColor(colYellow, colBg);
  if (mm < 10) tft.print("0");
  tft.print(mm);

  tft.setTextSize(4);
  tft.setTextColor(colWhite, colBg);
  tft.setCursor(184, 100);
  if (ss < 10) tft.print("0");
  tft.print(ss);
}

void drawFace1() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(colCyan, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(48, 18);
  tft.print("MINI CLOCK");

  tft.drawCircle(120, 126, 72, colWhite);
  tft.drawCircle(120, 126, 74, colWhite);
  for (int i = 0; i < 12; i++) {
    float a = (i * 30 - 90) * 0.0174533;
    int x1 = 120 + cos(a) * 58;
    int y1 = 126 + sin(a) * 58;
    int x2 = 120 + cos(a) * 66;
    int y2 = 126 + sin(a) * 66;
    tft.drawLine(x1, y1, x2, y2, colWhite);
  }
  tft.setCursor(72, 214);
  tft.setTextColor(colYellow, ST77XX_BLACK);
  tft.print("BTN = NEXT");
}

void updateFace1Hands() {
  tft.fillCircle(120, 126, 52, ST77XX_BLACK);
  for (int i = 0; i < 12; i++) {
    float a = (i * 30 - 90) * 0.0174533;
    int x1 = 120 + cos(a) * 58;
    int y1 = 126 + sin(a) * 58;
    int x2 = 120 + cos(a) * 66;
    int y2 = 126 + sin(a) * 66;
    tft.drawLine(x1, y1, x2, y2, colWhite);
  }
  float ah = ((hh % 12) * 30 + mm * 0.5 - 90) * 0.0174533;
  float am = (mm * 6 - 90) * 0.0174533;
  float as = (ss * 6 - 90) * 0.0174533;
  tft.drawLine(120, 126, 120 + cos(ah) * 30, 126 + sin(ah) * 30, colWhite);
  tft.drawLine(120, 126, 120 + cos(am) * 44, 126 + sin(am) * 44, colCyan);
  tft.drawLine(120, 126, 120 + cos(as) * 50, 126 + sin(as) * 50, colYellow);
  tft.fillCircle(120, 126, 4, colWhite);
}

void drawFace2() {
  tft.fillScreen(0x0841);
  tft.setTextColor(colYellow, 0x0841);
  tft.setTextSize(2);
  tft.setCursor(18, 16);
  tft.print("WEATHER PANEL");

  tft.fillRoundRect(16, 46, 208, 52, 10, ST77XX_BLACK);
  tft.setTextColor(colWhite, ST77XX_BLACK);
  tft.setCursor(28, 58);
  tft.print("Temp: "); tft.print(tempNow); tft.print("C");
  tft.setCursor(28, 78);
  tft.print("Humidity: "); tft.print(humidity); tft.print("%");

  tft.fillRoundRect(16, 110, 208, 88, 10, ST77XX_BLACK);
  drawCloud(34, 128);
  tft.fillRoundRect(122, 136, 76, 22, 5, colWhite);
  tft.setTextColor(colBadgeBlue, colWhite);
  tft.setCursor(132, 142);
  tft.print(weatherLabel);

  tft.setTextColor(colWhite, 0x0841);
  tft.setCursor(54, 212);
  tft.print("Press button to switch");
}

void drawCurrentFace() {
  if (face == 0) {
    drawStaticFace0();
    drawTimeBlock();
  } else if (face == 1) {
    drawFace1();
    updateFace1Hands();
  } else {
    drawFace2();
  }
}

void handleButton() {
  bool reading = digitalRead(BTN_PIN);
  if (reading != lastBtn) lastDebounce = millis();

  if ((millis() - lastDebounce) > 40) {
    static bool latched = HIGH;
    if (latched == HIGH && reading == LOW) {
      face = (face + 1) % 3;
      drawCurrentFace();
    }
    latched = reading;
  }
  lastBtn = reading;
}

void tickClock() {
  ss++;
  if (ss >= 60) { ss = 0; mm++; }
  if (mm >= 60) { mm = 0; hh++; }
  if (hh >= 24) hh = 0;
}

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 240);
  tft.setRotation(0);
  drawCurrentFace();
}

void loop() {
  handleButton();
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    tickClock();
    if (face == 0) drawTimeBlock();
    if (face == 1) updateFace1Hands();
  }
}
