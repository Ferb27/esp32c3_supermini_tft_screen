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
int hh = 14, mm = 51, ss = 18;
int dayNum = 26, monthNum = 5, yearNum = 2026;
int tempNow = 34;
int humidity = 57;
String city = "Ho Chi Minh";
String country = "VN";
String weatherLabel = "Hot";
String updatedText = "Updated 14:46";
const char* weekDay = "Tue";

uint16_t C_BG = ST77XX_BLACK;
uint16_t C_WHITE = ST77XX_WHITE;
uint16_t C_YELLOW = 0xFEE0;
uint16_t C_CYAN = 0x7FFF;
uint16_t C_GREEN = 0x07E0;
uint16_t C_BLUEBADGE = 0xC71F;
uint16_t C_THERMO_RED = 0xF8E3;
uint16_t C_BAR_GREEN = 0x5FE7;
uint16_t C_PAW = 0xFD20;

int mapTempBar(int tempC) {
  if (tempC < 20) tempC = 20;
  if (tempC > 40) tempC = 40;
  return map(tempC, 20, 40, 8, 40);
}

int mapHumBar(int hum) {
  if (hum < 0) hum = 0;
  if (hum > 100) hum = 100;
  return map(hum, 0, 100, 0, 44);
}

void drawSunTopBig(int x, int y) {
  tft.fillCircle(x + 18, y + 18, 13, C_YELLOW);
  for (int i = 0; i < 8; i++) {
    float a = i * 0.785398f;
    int x1 = x + 18 + cos(a) * 16;
    int y1 = y + 18 + sin(a) * 16;
    int x2 = x + 18 + cos(a) * 22;
    int y2 = y + 18 + sin(a) * 22;
    tft.drawLine(x1, y1, x2, y2, C_YELLOW);
  }
}

void drawThermoIconRef(int x, int y) {
  tft.drawRoundRect(x + 6, y + 1, 7, 21, 3, C_WHITE);
  tft.fillRect(x + 8, y + 7, 3, 10, C_THERMO_RED);
  tft.fillCircle(x + 9, y + 23, 7, C_THERMO_RED);
  tft.drawCircle(x + 9, y + 23, 7, C_WHITE);
  tft.drawLine(x + 16, y + 3, x + 16, y + 23, C_WHITE);
}

void drawHumidityIconRef(int x, int y) {
  tft.drawCircle(x + 10, y + 18, 8, C_CYAN);
  tft.fillCircle(x + 10, y + 18, 6, C_CYAN);
  tft.fillTriangle(x + 10, y + 3, x + 3, y + 14, x + 17, y + 14, C_CYAN);
  tft.drawCircle(x + 22, y + 21, 8, C_WHITE);
  tft.fillCircle(x + 22, y + 21, 5, C_WHITE);
  tft.fillTriangle(x + 22, y + 8, x + 15, y + 18, x + 29, y + 18, C_WHITE);
}

void drawBarWhiteBlue(int x, int y, int w, int fillW, uint16_t fillColor) {
  tft.drawRoundRect(x, y, w, 10, 4, C_WHITE);
  if (fillW > 0) tft.fillRoundRect(x + 2, y + 2, fillW, 6, 3, fillColor);
}

void drawPawIcon(int x, int y) {
  tft.fillCircle(x + 17, y + 24, 9, C_PAW);
  tft.fillCircle(x + 7,  y + 12, 4, C_PAW);
  tft.fillCircle(x + 14, y + 8, 4, C_PAW);
  tft.fillCircle(x + 22, y + 8, 4, C_PAW);
  tft.fillCircle(x + 29, y + 13, 4, C_PAW);
}

void drawStaticUI() {
  tft.fillScreen(C_BG);
  tft.setTextWrap(false);

  tft.setTextColor(C_YELLOW, C_BG);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print(city);

  tft.fillRoundRect(166, 7, 38, 22, 5, C_GREEN);
  tft.setTextColor(C_BG, C_GREEN);
  tft.setTextSize(2);
  tft.setCursor(174, 11);
  tft.print(country);

  drawSunTopBig(176, 38);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(10, 42);
  tft.print(updatedText);

  tft.fillRoundRect(150, 96, 50, 18, 4, C_WHITE);
  tft.setTextColor(C_BLUEBADGE, C_WHITE);
  tft.setTextSize(1);
  tft.setCursor(161, 102);
  tft.print(weatherLabel);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(18, 150);
  tft.print(dayNum); tft.print('/'); tft.print(monthNum); tft.print('/'); tft.print(yearNum);

  tft.setTextColor(C_GREEN, C_BG);
  tft.setTextSize(2);
  tft.setCursor(164, 150);
  tft.print(weekDay);

  drawThermoIconRef(10, 176);
  drawHumidityIconRef(10, 212);

  int tempBar = mapTempBar(tempNow);
  int humBar = mapHumBar(humidity);
  drawBarWhiteBlue(56, 186, 54, tempBar, 0x2D7F);
  drawBarWhiteBlue(56, 220, 54, humBar, C_BAR_GREEN);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(116, 184);
  tft.print(tempNow); tft.print("C");
  tft.setCursor(116, 218);
  tft.print(humidity); tft.print('%');

  drawPawIcon(170, 194);
}

void drawTime() {
  tft.fillRect(6, 66, 228, 76, C_BG);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(6);
  tft.setCursor(8, 78);
  if (hh < 10) tft.print('0');
  tft.print(hh);

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(5);
  tft.setCursor(76, 84);
  tft.print(':');

  tft.setTextColor(C_YELLOW, C_BG);
  tft.setTextSize(6);
  tft.setCursor(100, 78);
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
