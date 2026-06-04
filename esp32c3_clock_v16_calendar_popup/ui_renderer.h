#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "types.h"

class UIRenderer {
private:
  Adafruit_ST7789& _tft;
  GFXcanvas16* _clockCanvas; // Canvas đệm vùng Giờ để chống nháy giống hệt Sprite
  
  DateTimeData _prevTime; 
  WeatherData _prevWeather;
  
  void drawProgressBar(int x, int y, int w, int h, int percent, uint16_t color);
  void drawCyberDigit(GFXcanvas16* canvas, int x, int y, int val, uint16_t color, int w = 28, int h = 48, int t = 6);
  void drawCyberFrame(int x, int y, int w, int h, int cutSize, uint16_t color, uint16_t bgColor, bool fill = true);

public:
  UIRenderer(Adafruit_ST7789& tft);
  ~UIRenderer();
  
  void initScreen();
  void drawConnectingScreen(const char* ssid);
  void drawStaticLayout(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, const PetData* pet = nullptr);
  void updateDynamicUI(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, bool force = false, const PetData* pet = nullptr);
  void updateBongoAnimation(int frameState, int audioAmp = 0); // 0: Idle, 1: Left, 2: Right, 3: Both
  void drawCalendarOverlay(const DateTimeData& time); // Hàm vẽ calendar popup
};

#endif
