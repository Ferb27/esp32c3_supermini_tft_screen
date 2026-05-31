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
int hh = 14, mm = 35, ss = 57;
int dayNum = 26, monthNum = 5, yearNum = 2026;
int tempNow = 34;
int humidity = 57;
String city = "Ho Chi Minh";
String country = "VN";
String weatherLabel = "Clouds";
const char* weekDay = "Tue";

uint16_t C_BG = ST77XX_BLACK;
uint16_t C_WHITE = ST77XX_WHITE;
uint16_t C_YELLOW = 0xFEE0;
uint16_t C_CYAN = 0x7FFF;
uint16_t C_GREEN = 0x07E0;
uint16_t C_BLUEBADGE = 0xC71F;
uint16_t C_PINK = 0xFDF9;
uint16_t C_THERMO = 0xFBE0;

void drawCloudTop(int x, int y) {
  tft.fillCircle(x + 10, y + 16, 10, C_WHITE);
  tft.fillCircle(x + 24, y + 11, 13, C_WHITE);
  tft.fillCircle(x + 39, y + 17, 10, C_PINK);
  tft.fillRoundRect(x + 4, y + 18, 40, 13, 6, C_WHITE);
}

void drawBottomSun(int x, int y) {
  tft.fillCircle(x + 12, y + 16, 10, C_YELLOW);
  for (int i = 0; i < 8; i++) {
    float a = i * 0.785398f;
    int x1 = x + 12 + cos(a) * 13;
    int y1 = y + 16 + sin(a) * 13;
    int x2 = x + 12 + cos(a) * 17;
    int y2 = y + 16 + sin(a) * 17;
    tft.drawLine(x1, y1, x2, y2, C_YELLOW);
  }
  tft.fillCircle(x + 24, y + 21, 8, C_YELLOW);
  tft.fillCircle(x + 35, y + 19, 10, C_YELLOW);
  tft.fillRoundRect(x + 18, y + 22, 25, 10, 5, C_YELLOW);
}

void drawThermoIcon(int x, int y) {
  tft.drawCircle(x + 6, y + 16, 5, C_THERMO);
  tft.fillCircle(x + 6, y + 16, 4, C_THERMO);
  tft.drawRoundRect(x + 4, y, 4, 16, 2, C_THERMO);
  tft.fillRect(x + 5, y + 4, 2, 8, C_THERMO);
}

void drawDropIcon(int x, int y) {
  tft.fillTriangle(x + 6, y, x + 1, y + 9, x + 11, y + 9, C_CYAN);
  tft.fillCircle(x + 6, y + 11, 5, C_CYAN);
}

void drawStaticUI() {
  tft.fillScreen(C_BG);
  tft.setTextWrap(false);

  tft.setTextColor(C_YELLOW, C_BG);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print(city);

  tft.fillRoundRect(162, 8, 38, 20, 5, C_GREEN);
  tft.setTextColor(C_BG, C_GREEN);
  tft.setTextSize(1);
  tft.setCursor(173, 14);
  tft.print(country);

  drawCloudTop(178, 25);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(10, 38);
  tft.print("Max temp 30C");

  tft.fillRoundRect(150, 96, 62, 18, 4, C_WHITE);
  tft.setTextColor(C_BLUEBADGE, C_WHITE);
  tft.setTextSize(1);
  tft.setCursor(160, 102);
  tft.print(weatherLabel);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(18, 158);
  tft.print(dayNum); tft.print('/'); tft.print(monthNum); tft.print('/'); tft.print(yearNum);

  tft.setTextColor(C_GREEN, C_BG);
  tft.setCursor(170, 158);
  tft.print(weekDay);

  drawThermoIcon(16, 192);
  drawDropIcon(16, 212);

  tft.drawRoundRect(52, 198, 64, 10, 4, C_WHITE);
  tft.fillRoundRect(54, 200, 32, 6, 3, C_CYAN);
  tft.drawRoundRect(52, 218, 64, 10, 4, C_WHITE);
  tft.fillRoundRect(54, 220, 36, 6, 3, C_CYAN);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(122, 194);
  tft.print(tempNow); tft.print("C");
  tft.setCursor(122, 214);
  tft.print(humidity); tft.print('%');

  drawBottomSun(178, 193);
}

void drawTime() {
  tft.fillRect(6, 66, 228, 76, C_BG);

  tft.setTextSize(6);
  tft.setTextColor(C_WHITE, C_BG);
  tft.setCursor(8, 78);
  if (hh < 10) tft.print('0');
  tft.print(hh);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(5);
  tft.setCursor(74, 84);
  tft.print(':');

  tft.setTextColor(C_YELLOW, C_BG);
  tft.setTextSize(6);
  tft.setCursor(98, 78);
  if (mm < 10) tft.print('0');
  tft.print(mm);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(3);
  tft.setCursor(184, 99);
  if (ss < 10) tft.print('0');
  tft.print(ss);
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
  drawStaticUI();
  drawTime();
}

void loop() {
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    tickClock();
    drawTime();
  }
}
