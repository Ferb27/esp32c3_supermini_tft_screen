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
int hh = 14, mm = 24, ss = 45;
int dayNum = 26, monthNum = 05, yearNum = 2026;
const char* weekDay = "Fri";
int tempNow = 34;
int humidity = 57;
const char* city = "Ho Chi Minh";
const char* country = "VN";
const char* weatherLabel = "Sun";

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
  tft.fillCircle(x + 14, y + 17, 11, C_YELLOW);
  for (int i = 0; i < 8; i++) {
    float a = i * 0.785398f;
    int x1 = x + 14 + cos(a) * 14;
    int y1 = y + 17 + sin(a) * 14;
    int x2 = x + 14 + cos(a) * 18;
    int y2 = y + 17 + sin(a) * 18;
    tft.drawLine(x1, y1, x2, y2, C_YELLOW);
  }
  tft.fillCircle(x + 27, y + 22, 9, C_YELLOW);
  tft.fillCircle(x + 39, y + 20, 11, C_YELLOW);
  tft.fillRoundRect(x + 20, y + 23, 28, 11, 5, C_YELLOW);
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
  tft.setCursor(12, 10);
  tft.print(city);

  tft.fillRoundRect(142, 8, 42, 22, 5, C_GREEN);
  tft.setTextColor(C_BG, C_GREEN);
  tft.setCursor(153, 14);
  tft.print(country);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setCursor(12, 38);
  tft.print("Max temp 30C");

  drawCloudTop(160, 28);

  tft.fillRoundRect(148, 93, 66, 20, 5, C_WHITE);
  tft.setTextColor(C_BLUEBADGE, C_WHITE);
  tft.setTextSize(1);
  tft.setCursor(160, 99);
  tft.print(weatherLabel);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(16, 160);
  tft.print(dayNum); tft.print('/'); tft.print(monthNum); tft.print('/'); tft.print(yearNum);

  tft.setTextColor(C_GREEN, C_BG);
  tft.setCursor(168, 160);
  tft.print(weekDay);

  drawThermoIcon(14, 192);
  drawDropIcon(14, 212);

  tft.drawRoundRect(56, 198, 72, 10, 4, C_WHITE);
  tft.fillRoundRect(58, 200, 38, 6, 3, C_CYAN);
  tft.drawRoundRect(56, 218, 72, 10, 4, C_WHITE);
  tft.fillRoundRect(58, 220, 43, 6, 3, C_CYAN);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setCursor(134, 194);
  tft.print(tempNow); tft.print("C");
  tft.setCursor(134, 214);
  tft.print(humidity); tft.print('%');

  drawBottomSun(175, 191);
}

void drawTime() {
  tft.fillRect(6, 66, 228, 76, C_BG);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(6);
  tft.setCursor(10, 78);
  if (hh < 10) tft.print('0');
  tft.print(hh);

  tft.setCursor(80, 78);
  tft.print(':');

  tft.setTextColor(C_YELLOW, C_BG);
  tft.setCursor(105, 78);
  if (mm < 10) tft.print('0');
  tft.print(mm);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(3);
  tft.setCursor(186, 99);
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
