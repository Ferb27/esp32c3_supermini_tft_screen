#include "ui_renderer.h"
#include "icon_manager.h"
#include "config.h"

UIRenderer::UIRenderer(Adafruit_ST7789& tft) : _tft(tft) {
  _clockCanvas = new GFXcanvas16(220, 60);
  _prevTime = {-1, -1, -1, -1, -1, -1, ""};
  _prevWeather = {-999, -1, "", "", -1, ""};
}

UIRenderer::~UIRenderer() {
  delete _clockCanvas;
}

void UIRenderer::initScreen() {
  _tft.fillScreen(0x0000);
}

void UIRenderer::drawConnectingScreen(const char* ssid) {
  _tft.fillScreen(0x0000);
  
  // Background grid
  for (int i = 0; i < 240; i += 30) {
    _tft.drawFastHLine(0, i, 240, 0x0841); 
    _tft.drawFastVLine(i, 0, 240, 0x0841);
  }
  
  // Outer frame
  drawCyberFrame(10, 30, 220, 180, 15, 0x07FF, 0x0000, false);
  
  // Decorative lines
  _tft.drawFastHLine(25, 30, 50, 0xF81F);
  _tft.drawFastHLine(165, 210, 50, 0xF81F);
  
  _tft.setTextSize(2);
  
  _tft.setTextColor(0xF81F); // Pink Neon
  _tft.setCursor(30, 55);
  _tft.print("SYSTEM UPLINK");
  
  _tft.setTextColor(0x07FF); // Cyan
  _tft.setCursor(30, 90);
  _tft.print("CONNECTING...");
  
  _tft.setTextColor(0xFE60); // Cyber Yellow
  _tft.setCursor(30, 125);
  _tft.printf("> SSID: %s", ssid);
  
  _tft.setTextColor(0xFFFF); // White
  _tft.setCursor(30, 165);
  _tft.print("WAIT DHCP...");
}

void UIRenderer::drawProgressBar(int x, int y, int w, int h, int percent, uint16_t color) {
  _tft.drawRoundRect(x, y, w, h, h/2, 0xFFFF);
  int fillW = map(constrain(percent, 0, 100), 0, 100, 0, w - 4);
  _tft.fillRect(x + 2, y + 2, w - 4, h - 4, 0x0000); 
  if (fillW > 0) {
    _tft.fillRoundRect(x + 2, y + 2, fillW, h - 4, (h-4)/2, color);
  }
}

void UIRenderer::drawCyberDigit(GFXcanvas16* canvas, int x, int y, int val, uint16_t color, int w, int h, int t) {
  if (val < 0 || val > 9) return;
  const uint8_t seg[10] = {
    0b00111111, // 0: A,B,C,D,E,F
    0b00000110, // 1: B,C
    0b01011011, // 2: A,B,D,E,G
    0b01001111, // 3: A,B,C,D,G
    0b01100110, // 4: B,C,F,G
    0b01101101, // 5: A,C,D,F,G
    0b01111101, // 6: A,C,D,E,F,G
    0b00000111, // 7: A,B,C
    0b01111111, // 8: A,B,C,D,E,F,G
    0b01101111  // 9: A,B,C,D,F,G
  };
  
  uint8_t s = seg[val];
  int g = 1; // Khoảng cách giữa các nét
  
  // A (Top)
  if (s & 0x01) canvas->fillRect(x+t+g, y, w-2*t-2*g, t, color);
  // B (Top Right)
  if (s & 0x02) canvas->fillRect(x+w-t, y+t+g, t, h/2-t-2*g, color);
  // C (Bottom Right)
  if (s & 0x04) canvas->fillRect(x+w-t, y+h/2+g, t, h/2-t-2*g, color);
  // D (Bottom)
  if (s & 0x08) canvas->fillRect(x+t+g, y+h-t, w-2*t-2*g, t, color);
  // E (Bottom Left)
  if (s & 0x10) canvas->fillRect(x, y+h/2+g, t, h/2-t-2*g, color);
  // F (Top Left)
  if (s & 0x20) canvas->fillRect(x, y+t+g, t, h/2-t-2*g, color);
  // G (Middle)
  if (s & 0x40) canvas->fillRect(x+t+g, y+h/2-t/2, w-2*t-2*g, t, color);
}

void UIRenderer::drawCyberFrame(int x, int y, int w, int h, int cutSize, uint16_t color, uint16_t bgColor, bool fill) {
  if (fill) {
    _tft.fillRect(x + cutSize, y, w - 2*cutSize, h, bgColor);
    _tft.fillRect(x, y + cutSize, w, h - 2*cutSize, bgColor);
    _tft.fillTriangle(x, y+cutSize, x+cutSize, y, x+cutSize, y+cutSize, bgColor);
    _tft.fillTriangle(x+w-cutSize-1, y, x+w-1, y+cutSize, x+w-cutSize-1, y+cutSize, bgColor);
    _tft.fillTriangle(x, y+h-cutSize-1, x+cutSize, y+h-1, x+cutSize, y+h-cutSize-1, bgColor);
    _tft.fillTriangle(x+w-1, y+h-cutSize-1, x+w-cutSize-1, y+h-1, x+w-cutSize-1, y+h-cutSize-1, bgColor);
  }
  
  _tft.drawLine(x + cutSize, y, x + w - cutSize - 1, y, color); // top
  _tft.drawLine(x + w - 1, y + cutSize, x + w - 1, y + h - cutSize - 1, color); // right
  _tft.drawLine(x + cutSize, y + h - 1, x + w - cutSize - 1, y + h - 1, color); // bottom
  _tft.drawLine(x, y + cutSize, x, y + h - cutSize - 1, color); // left
  
  _tft.drawLine(x, y + cutSize, x + cutSize, y, color); // top left
  _tft.drawLine(x + w - cutSize - 1, y, x + w - 1, y + cutSize, color); // top right
  _tft.drawLine(x, y + h - cutSize - 1, x + cutSize, y + h - 1, color); // bottom left
  _tft.drawLine(x + w - 1, y + h - cutSize - 1, x + w - cutSize - 1, y + h - 1, color); // bottom right
}

void UIRenderer::drawStaticLayout(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, const PetData* pet) {
  _tft.fillScreen(0x0000);
  
  if (mode == LAYOUT_FULL) {
    _tft.setTextSize(2);
    int cityPxW = weather.city.length() * 12;
    int badgeW = weather.country.length() * 12 + 8;
    int totalW = cityPxW + 6 + badgeW;
    int startX = (240 - totalW) / 2;
    if (startX < 2) startX = 2;
    
    _tft.setTextColor(0xFEE0);
    _tft.setCursor(startX, 10);
    _tft.print(weather.city);
    
    int badgeX = startX + cityPxW + 6;
    _tft.fillRoundRect(badgeX, 8, badgeW, 20, 4, 0x07E0);
    _tft.setTextColor(0x0000);
    _tft.setCursor(badgeX + 4, 11);
    _tft.print(weather.country);
    
    _tft.drawFastHLine(10, 36, 220, 0x31A6); 
    
    _tft.setTextColor(0xFFFF);
    _tft.setTextSize(2);
    _tft.setCursor(10, 44);
    _tft.print("Updated: " + weather.lastUpdated);
    
    IconManager::drawWeatherIcon(_tft, 185, 38, weather.weatherCode, time.hour);
    
    _tft.setTextColor(0xFFFF);
    _tft.setTextSize(2);
    _tft.setCursor(15, 145);
    _tft.printf("%02d/%02d/%04d", time.day, time.month, time.year);
    
    _tft.setTextColor(0x07E0);
    _tft.setCursor(170, 145);
    _tft.print(time.dayOfWeek);
    
    IconManager::drawStaticIcon(_tft, 10, 175, ICON_TYPE_TEMP, weather.temp);
    _tft.setTextColor(0xFFFF);
    _tft.setTextSize(2);
    _tft.setCursor(120, 180);
    _tft.print(weather.temp); _tft.print("C");
    
    IconManager::drawStaticIcon(_tft, 10, 210, ICON_TYPE_HUMIDITY, weather.humidity);
    _tft.setCursor(120, 215);
    _tft.print(weather.humidity); _tft.print("%");
    
    int tempPercent = map(constrain(weather.temp, 15, 45), 15, 45, 0, 100);
    drawProgressBar(55, 184, 60, 10, tempPercent, 0xF800);
    drawProgressBar(55, 219, 60, 10, weather.humidity, 0x07E0);
    
    IconManager::drawStaticIcon(_tft, 185, 185, ICON_TYPE_DECOR, 0);
  } 
  else if (mode == LAYOUT_MINIMAL) {
    _tft.drawFastVLine(115, 20, 200, 0x31A6);
    IconManager::drawWeatherIcon(_tft, 155, 30, weather.weatherCode, time.hour);
  }
  else if (mode == LAYOUT_CYBERPUNK) {
    for (int i = 0; i < 240; i += 15) {
      _tft.drawFastHLine(0, i, 240, 0x0841); 
      _tft.drawFastVLine(i, 0, 240, 0x0841);
    }
    
    drawCyberFrame(0, 0, 240, 240, 20, 0x07FF, 0x0000, false);
    
    drawCyberFrame(15, -5, 210, 34, 8, 0x07FF, 0x07FF, true);
    _tft.setTextColor(0x0000); 
    _tft.setTextSize(2);
    _tft.setCursor(55, 8);
    _tft.print("NEURAL LINK");

    _tft.setTextColor(0xF81F);
    _tft.setTextSize(1);
    _tft.setCursor(5, 31);
    _tft.print("SEC_09");
    _tft.setCursor(195, 31);
    _tft.print("OVERRIDE");

    _tft.setTextColor(0x07FF);
    for (int i=0; i<6; i++) {
      _tft.setCursor(5, 60 + i*15);
      _tft.print(i % 2 == 0 ? "1" : "0");
      _tft.setCursor(230, 60 + i*15);
      _tft.print(i % 2 == 0 ? "0" : "1");
    }
    
    _tft.drawLine(120, 35, 120, 65, 0xF81F);
    _tft.drawLine(115, 50, 125, 50, 0xF81F);

    drawCyberFrame(8, 178, 105, 54, 8, 0x07FF, 0x2104, true);
    _tft.setTextColor(0x07FF);
    _tft.setTextSize(2);
    _tft.setCursor(19, 184);
    _tft.print("ENV.TMP");
    
    drawCyberFrame(127, 178, 105, 54, 8, 0x07FF, 0x2104, true);
    _tft.setCursor(138, 184);
    _tft.print("ENV.HUM");
    
    _tft.fillTriangle(120, 178, 115, 188, 125, 188, 0xF81F);
    _tft.fillTriangle(120, 232, 115, 222, 125, 222, 0xF81F);
  }
  else if (mode == LAYOUT_BONGO_CAT) {
    _tft.fillScreen(0x0000); 
    _tft.drawRoundRect(10, 10, 80, 30, 5, 0x07FF);
    updateBongoAnimation(0, 0);
  }
}

void UIRenderer::updateDynamicUI(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, bool force, const PetData* pet) {
  if (force || time.second != _prevTime.second) {
    _clockCanvas->fillScreen(0x0000); 
    
    if (mode == LAYOUT_FULL) {
      _clockCanvas->setTextColor(0xFFFF);
      _clockCanvas->setTextSize(6);
      
      _clockCanvas->setCursor(5, 5);
      if (time.hour < 10) _clockCanvas->print('0');
      _clockCanvas->print(time.hour);
      
      _clockCanvas->setTextSize(6);
      _clockCanvas->setCursor(72, 5);
      if (time.second % 2 == 0) _clockCanvas->setTextColor(0xFEE0);
      else _clockCanvas->setTextColor(0x0000);
      _clockCanvas->print(':');
      
      _clockCanvas->setTextColor(0xFEE0);
      _clockCanvas->setTextSize(6);
      _clockCanvas->setCursor(107, 5);
      if (time.minute < 10) _clockCanvas->print('0');
      _clockCanvas->print(time.minute);
      
      _clockCanvas->setTextColor(0xFFFF);
      _clockCanvas->setTextSize(3);
      _clockCanvas->setCursor(182, 26);
      if (time.second < 10) _clockCanvas->print('0');
      _clockCanvas->print(time.second);
      
      _tft.drawRGBBitmap(10, 78, _clockCanvas->getBuffer(), 220, 60);
    } 
    else if (mode == LAYOUT_MINIMAL) {
      if (force || time.minute != _prevTime.minute || time.hour != _prevTime.hour) {
        _tft.fillRect(15, 55, 85, 120, 0x0000);
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(6);
        _tft.setCursor(21, 60);
        if (time.hour < 10) _tft.print('0');
        _tft.print(time.hour);
        
        _tft.setTextColor(0xFEE0);
        _tft.setCursor(21, 120);
        if (time.minute < 10) _tft.print('0');
        _tft.print(time.minute);
      }
      
      if (force || weather.temp != _prevWeather.temp) {
        _tft.fillRect(130, 90, 95, 26, 0x0000);
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(3);
        _tft.setCursor(148, 90);
        _tft.print(weather.temp); _tft.print((char)247); 
      }
      
      if (force || time.day != _prevTime.day) {
        _tft.fillRect(130, 140, 95, 50, 0x0000);
        _tft.setTextColor(0x07FF);
        _tft.setTextSize(2);
        _tft.setCursor(157, 140);
        _tft.print(time.dayOfWeek);
        
        _tft.setTextColor(0xFFFF);
        _tft.setCursor(145, 165);
        _tft.printf("%02d/%02d", time.day, time.month);
      }
      
      if (force || weather.humidity != _prevWeather.humidity) {
        _tft.fillRect(130, 195, 95, 20, 0x0000);
        _tft.setTextColor(0x07E0);
        _tft.setTextSize(2);
        _tft.setCursor(157, 195);
        _tft.print(weather.humidity); _tft.print("%");
      }
    }
    else if (mode == LAYOUT_CYBERPUNK) {
      _clockCanvas->fillScreen(0x0000);
      
      _tft.fillRect(0, 42, 240, 16, 0x0000); 
      _tft.setTextSize(2);
      String displayCity = weather.city;
      if (displayCity.length() > 12) displayCity = displayCity.substring(0, 12);
      String locText = displayCity + " " + weather.country;
      int locPxW = locText.length() * 12;
      int locX = (240 - locPxW) / 2;
      if (locX < 2) locX = 2;
      _tft.setCursor(locX, 42);
      _tft.setTextColor(0x07E0);
      _tft.print(displayCity);
      _tft.setTextColor(0xFE60);
      _tft.print(" ");
      _tft.print(weather.country);

      _tft.fillRect(180, 145, 40, 20, 0x0000);
      for (int i = 0; i < 6; i++) {
        int h = random(2, 18);
        _tft.fillRect(180 + i*6, 165 - h, 4, h, 0x07FF);
      }
      
      _tft.fillRect(10, 146, 132, 16, 0x0000);
      _tft.setTextSize(2);
      if (time.second % 2 == 0) {
        _tft.setTextColor(0xF81F);
        _tft.setCursor(10, 146);
        _tft.print("SYS.WARNING");
      } else {
        _tft.setTextColor(0x07FF);
        _tft.setCursor(10, 146);
        _tft.print("SYS.STABLE");
      }

      int h_t = time.hour / 10;
      int h_u = time.hour % 10;
      int m_t = time.minute / 10;
      int m_u = time.minute % 10;
      
      int y = 5;
      drawCyberDigit(_clockCanvas, 15, y, h_t, 0xFE60); 
      drawCyberDigit(_clockCanvas, 51, y, h_u, 0xFE60);
      
      if (time.second % 2 == 0) {
        _clockCanvas->fillRect(92, y+10, 6, 6, 0xF81F);
        _clockCanvas->fillRect(92, y+32, 6, 6, 0xF81F);
        
        _clockCanvas->setTextColor(0xF800); 
        _clockCanvas->setTextSize(1);
        _clockCanvas->setCursor(0, 0);
        _clockCanvas->print("[REC]");
      }
      
      drawCyberDigit(_clockCanvas, 111, y, m_t, 0xFE60);
      drawCyberDigit(_clockCanvas, 147, y, m_u, 0xFE60);
      
      int s_t = time.second / 10;
      int s_u = time.second % 10;
      drawCyberDigit(_clockCanvas, 182, y+24, s_t, 0x07FF, 14, 24, 3);
      drawCyberDigit(_clockCanvas, 200, y+24, s_u, 0x07FF, 14, 24, 3);
      
      int totalSegments = 40; 
      int activeSegments = map(time.second, 0, 59, 0, totalSegments);
      int segWidth = 3;
      int segGap = 2;
      int startX = 10;
      
      for (int i = 0; i < totalSegments; i++) {
        uint16_t segColor = (i <= activeSegments) ? 0xF81F : 0x2104;
        _clockCanvas->fillRect(startX + i * (segWidth + segGap), 56, segWidth, 4, segColor);
      }
      
      _tft.drawRGBBitmap(10, 75, _clockCanvas->getBuffer(), 220, 60);
    }
    else if (mode == LAYOUT_BONGO_CAT) {
      _tft.fillRect(12, 12, 76, 26, 0x0000);
      _tft.setTextColor(0xFFFF);
      _tft.setTextSize(2);
      _tft.setCursor(15, 17);
      if (time.hour < 10) _tft.print('0');
      _tft.print(time.hour);
      if (time.second % 2 == 0) _tft.print(':'); else _tft.print(' ');
      if (time.minute < 10) _tft.print('0');
      _tft.print(time.minute);
      
      if (pet != nullptr) {
        _tft.fillRect(95, 10, 135, 30, 0x0000);
        _tft.setTextColor(0xFE60); 
        _tft.setTextSize(2);
        _tft.setCursor(95, 17);
        _tft.print(pet->todayKeystrokes);
        _tft.setTextSize(1);
        _tft.print(" keys");
      }
    }
  }

  if (mode == LAYOUT_FULL) {
    if (force || time.day != _prevTime.day) {
      _tft.setTextColor(0xFFFF, 0x0000);
      _tft.setTextSize(2);
      _tft.setCursor(15, 145);
      _tft.printf("%02d/%02d/%04d", time.day, time.month, time.year);
      
      _tft.setTextColor(0x07E0, 0x0000);
      _tft.fillRect(170, 145, 60, 16, 0x0000); 
      _tft.setCursor(170, 145);
      _tft.print(time.dayOfWeek);
    }
    
    if (force || weather.temp != _prevWeather.temp) {
      _tft.setTextColor(0xFFFF, 0x0000);
      _tft.setTextSize(2);
      _tft.setCursor(120, 180);
      _tft.print(weather.temp); _tft.print("C  "); 
      int tempPercent = map(constrain(weather.temp, 15, 45), 15, 45, 0, 100);
      drawProgressBar(55, 184, 60, 10, tempPercent, 0xF800);
    }

    if (force || weather.humidity != _prevWeather.humidity) {
      _tft.setTextColor(0xFFFF, 0x0000);
      _tft.setTextSize(2);
      _tft.setCursor(120, 215);
      _tft.print(weather.humidity); _tft.print("%  "); 
      drawProgressBar(55, 219, 60, 10, weather.humidity, 0x07E0);
    }
  }
  else if (mode == LAYOUT_CYBERPUNK) {
    if (force || weather.temp != _prevWeather.temp) {
      _tft.fillRect(12, 201, 97, 26, 0x2104);
      _tft.setTextColor(0xFE60, 0x2104); 
      _tft.setTextSize(3);
      _tft.setCursor(30, 203);
      _tft.print(weather.temp); 
      _tft.setTextSize(2);
      _tft.print("C");
    }
    if (force || weather.humidity != _prevWeather.humidity) {
      _tft.fillRect(131, 201, 97, 26, 0x2104);
      _tft.setTextColor(0x07FF, 0x2104); 
      _tft.setTextSize(3);
      _tft.setCursor(155, 203);
      _tft.print(weather.humidity);
      _tft.setTextSize(2);
      _tft.print("%");
    }
  }
  else if (mode == LAYOUT_BONGO_CAT && pet != nullptr) {
    if (force) {
      _tft.fillRect(0, 150, 240, 90, 0x0000); 
      if (pet->currentStreak > 1) {
        uint16_t streakColor = 0xF800; 
        if (pet->currentStreak > 30) streakColor = 0xFE60; 
        if (pet->currentStreak > 80) streakColor = 0x07FF; 
        if (pet->currentStreak > 150) streakColor = 0xF81F; 
        
        int offsetX = (pet->currentStreak > 15) ? random(-3, 4) : 0;
        int offsetY = (pet->currentStreak > 15) ? random(-3, 4) : 0;
        
        _tft.setTextColor(streakColor);
        _tft.setTextSize(3);
        
        String comboText = "COMBO x" + String(pet->currentStreak);
        int textWidth = comboText.length() * 18; 
        int textX = (240 - textWidth) / 2 + offsetX;
        if (textX < 0) textX = 0;
        
        _tft.setCursor(textX, 170 + offsetY);
        _tft.print(comboText);
        
        _tft.setTextColor(0x07E0); 
        _tft.setTextSize(1);
        String maxText = "MAX COMBO: " + String(pet->maxStreak);
        int maxW = maxText.length() * 6;
        _tft.setCursor((240 - maxW)/2, 210);
        _tft.print(maxText);
      } else {
        _tft.setTextColor(0x7BEF); 
        _tft.setTextSize(2);
        String maxText = "MAX: " + String(pet->maxStreak);
        int maxW = maxText.length() * 12;
        _tft.setCursor((240 - maxW)/2, 180);
        _tft.print(maxText);
      }
    }
  }
  
  _prevTime = time;
  _prevWeather = weather;
}

// Helper: vẽ hình thoi bầu bầu
static void drawPaw(Adafruit_GFX& tft, int cx, int cy, int w, int h, uint16_t outlineColor, uint16_t fillColor) {
  tft.fillCircle(cx, cy, w/2 + 2, outlineColor);
  tft.fillTriangle(cx - w/2 - 2, cy, cx, cy - h/2 - 2, cx + w/2 + 2, cy, outlineColor); 
  tft.fillTriangle(cx - w/2 - 2, cy, cx, cy + h/2 + 2, cx + w/2 + 2, cy, outlineColor); 
  
  tft.fillCircle(cx, cy, w/2, fillColor);
  tft.fillTriangle(cx - w/2, cy, cx, cy - h/2, cx + w/2, cy, fillColor); 
  tft.fillTriangle(cx - w/2, cy, cx, cy + h/2, cx + w/2, cy, fillColor); 
}

void UIRenderer::updateBongoAnimation(int frameState, int audioAmp) {
  if (frameState < 0 || frameState > 3) frameState = 0;
  
  // Allocate a half-height canvas (240x58 = 27KB) to save RAM and prevent crashes
  static GFXcanvas16* canvas = nullptr;
  if (!canvas) {
    canvas = new GFXcanvas16(240, 58);
    if (canvas && !canvas->getBuffer()) {
      delete canvas;
      canvas = nullptr;
    }
  }
  
  if (!canvas) {
    return; // Safety fallback if memory is completely full
  }

  // Pre-calculate randomized values so both chunks look identical
  int numBars = 10;
  int baseHeight = map(constrain(audioAmp, 0, 200), 0, 200, 0, 95);
  int barHeights[10];
  for (int i = 0; i < numBars; i++) {
    float factor = 1.0;
    if (i == 0 || i == 9) factor = 0.3;
    else if (i == 1 || i == 8) factor = 0.5;
    else if (i == 2 || i == 7) factor = 0.75;
    else if (i == 3 || i == 6) factor = 0.9;
    
    int barH = (int)(baseHeight * factor);
    barH += random(-4, 5);
    barHeights[i] = constrain(barH, 0, 100);
  }
  
  int bob = map(audioAmp, 0, 255, 0, 5); 

  auto drawScene = [&](Adafruit_GFX* gfx, int offsetY) {
    gfx->fillScreen(0x0000);
    int tableY = 125 - offsetY;
    
    // === 0. DRAW EQUALIZER BARS ===
    if (audioAmp > 0) {
      int startX = 14;
      int barW = 16;
      int gap = 6;
      
      for (int i = 0; i < numBars; i++) {
        int barH = barHeights[i];
        if (barH > 0) {
          int bx = startX + i * (barW + gap);
          int by = tableY - barH;
          
          uint16_t color = 0x07FF;
          if (barH > 70) color = 0xF81F;
          else if (barH > 40) color = 0xFE60;
          
          gfx->fillRect(bx, by, barW, barH, color);
        }
      }
    }

    // === 1. CƠ THỂ MÈO ===
    int catX = 120;
    int catY = 85 + bob - offsetY; 
    
    gfx->fillCircle(catX, catY, 46, 0x0000);
    gfx->fillRect(catX - 46, catY, 92, tableY - catY, 0x0000);
    
    gfx->fillTriangle(catX - 42, catY - 15, catX - 25, catY - 48, catX - 8, catY - 35, 0x0000);
    gfx->fillTriangle(catX + 42, catY - 15, catX + 25, catY - 48, catX + 8, catY - 35, 0x0000);
    
    gfx->fillCircle(catX, catY, 43, 0xFFFF);
    gfx->fillRect(catX - 43, catY, 86, tableY - catY, 0xFFFF);
    
    gfx->fillTriangle(catX - 38, catY - 17, catX - 25, catY - 44, catX - 11, catY - 34, 0xFFFF);
    gfx->fillTriangle(catX + 38, catY - 17, catX + 25, catY - 44, catX + 11, catY - 34, 0xFFFF);
    
    gfx->fillCircle(catX - 24, catY + 2, 4, 0x0000);
    gfx->fillCircle(catX + 24, catY + 2, 4, 0x0000);
    
    gfx->drawLine(catX - 6, catY + 6, catX - 3, catY + 10, 0x0000);
    gfx->drawLine(catX - 3, catY + 10, catX, catY + 6, 0x0000);
    gfx->drawLine(catX, catY + 6, catX + 3, catY + 10, 0x0000);
    gfx->drawLine(catX + 3, catY + 10, catX + 6, catY + 6, 0x0000);
    
    gfx->drawLine(catX - 6, catY + 7, catX - 3, catY + 11, 0x0000);
    gfx->drawLine(catX - 3, catY + 11, catX, catY + 7, 0x0000);
    gfx->drawLine(catX, catY + 7, catX + 3, catY + 11, 0x0000);
    gfx->drawLine(catX + 3, catY + 11, catX + 6, catY + 7, 0x0000);

    // === 2. MẶT BÀN & BÀN PHÍM ===
    gfx->fillRect(0, tableY, 240, (147 - offsetY) - tableY, 0x2124); 
    gfx->drawLine(0, tableY, 240, tableY, 0xFFFF); 
    
    int kbX = 30, kbY = tableY - 12, kbW = 180, kbH = 32;
    gfx->fillRoundRect(kbX, kbY, kbW, kbH, 6, 0x0000); 
    gfx->fillRoundRect(kbX+2, kbY+2, kbW-4, kbH-4, 4, 0xBDD7);
    
    int keyW = 12, keyH = 10;
    for(int row=0; row<2; row++) {
      for(int col=0; col<10; col++) {
        int kx = kbX + 10 + col * (keyW + 3) + (row * 6);
        int ky = kbY + 4 + row * (keyH + 4);
        gfx->fillRect(kx, ky, keyW, keyH, 0x0000); 
        gfx->fillRect(kx+1, ky+1, keyW-2, keyH-2, 0xFFFF); 
      }
    }

    // === 3. BÀN TAY ===
    bool leftDown  = (frameState == 1 || frameState == 3);
    bool rightDown = (frameState == 2 || frameState == 3);
    uint16_t pinkColor = 0xFBEF;
    int pawW = 20, pawH = 26; 
    
    int lpX = catX - 48;
    if (leftDown) {
      drawPaw(*gfx, lpX, kbY + 6, pawW, pawH, 0x0000, 0xFFFF);
    } else {
      int lpY = tableY - 20;
      drawPaw(*gfx, lpX, lpY, pawW, pawH, 0x0000, 0xFFFF);
      gfx->fillCircle(lpX, lpY + 2, 5, pinkColor);     
      gfx->fillCircle(lpX - 5, lpY - 4, 2, pinkColor);  
      gfx->fillCircle(lpX,     lpY - 6, 2, pinkColor);  
      gfx->fillCircle(lpX + 5, lpY - 4, 2, pinkColor);  
    }
    
    int rpX = catX + 48;
    if (rightDown) {
      drawPaw(*gfx, rpX, kbY + 6, pawW, pawH, 0x0000, 0xFFFF);
    } else {
      int rpY = tableY - 20;
      drawPaw(*gfx, rpX, rpY, pawW, pawH, 0x0000, 0xFFFF);
      gfx->fillCircle(rpX, rpY + 2, 5, pinkColor);
      gfx->fillCircle(rpX - 5, rpY - 4, 2, pinkColor);
      gfx->fillCircle(rpX,     rpY - 6, 2, pinkColor);
      gfx->fillCircle(rpX + 5, rpY - 4, 2, pinkColor);
    }
  };

  // Render Top Half (Y: 32 to 89)
  drawScene(canvas, 32);
  _tft.drawRGBBitmap(0, 32, canvas->getBuffer(), 240, 58);
  
  // Render Bottom Half (Y: 90 to 146)
  drawScene(canvas, 90);
  _tft.drawRGBBitmap(0, 90, canvas->getBuffer(), 240, 57);
}

// === LỊCH THU NHỎ (CALENDAR POPUP) ===
int getDayOfWeek(int d, int m, int y) {
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  y -= m < 3;
  return ( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

int getDaysInMonth(int m, int y) {
  if (m == 2) {
    return ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ? 29 : 28;
  }
  if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
  return 31;
}

void UIRenderer::drawCalendarOverlay(const DateTimeData& time) {
  int calWidth = 236;
  int calHeight = 236;
  int startX = 2;
  int startY = 2;
  
  // 1. Soft Drop Shadow (Tạo độ sâu nhiều lớp)
  _tft.fillRoundRect(startX + 3, startY + 3, calWidth, calHeight, 14, 0x0000); 
  _tft.fillRoundRect(startX + 1, startY + 1, calWidth, calHeight, 14, 0x0841); 
  
  // 2. Main Glass Background (Deep Liquid Navy/Charcoal)
  uint16_t bgColor = 0x1104; // Tông nền xanh đen siêu tối, tạo cảm giác kính mờ (Frosted Dark Glass)
  _tft.fillRoundRect(startX, startY, calWidth, calHeight, 14, bgColor); 
  
  // 3. 3D Bevel & Glass Edge Highlights (Hiệu ứng viền nổi và độ dày của kính)
  // Viền sáng bắt sáng viền ngoài cùng
  _tft.drawRoundRect(startX, startY, calWidth, calHeight, 14, 0x4A69); 
  // Rãnh tối tạo độ sâu (Độ dày kính)
  _tft.drawRoundRect(startX + 1, startY + 1, calWidth - 2, calHeight - 2, 13, 0x0000);
  // Viền hắt sáng nhẹ bên trong
  _tft.drawRoundRect(startX + 2, startY + 2, calWidth - 4, calHeight - 4, 12, 0x2945);

  // 4. Liquid Shine (Vệt phản quang ánh sáng ở cạnh trên cùng)
  _tft.drawFastHLine(startX + 20, startY + 3, calWidth - 40, 0x7BCF); // Vệt sáng mờ
  _tft.drawFastHLine(startX + 30, startY + 4, calWidth - 60, 0xCE59); // Vệt sáng gắt (Glossy highlight)

  // 5. Header: Month and Year
  const char* monthNames[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
                              "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  int mIdx = constrain(time.month - 1, 0, 11);
  
  _tft.setTextColor(0xFFFF); 
  _tft.setTextSize(3); // Large font for premium feel
  String headerText = String(monthNames[mIdx]) + " " + String(time.year);
  int textW = headerText.length() * 18;
  _tft.setCursor(startX + (calWidth - textW) / 2, startY + 16);
  _tft.print(headerText);
  
  // Subtle separator line
  _tft.drawFastHLine(startX + 20, startY + 48, calWidth - 40, 0x39E7); 
  
  // 4. Days of the week headers
  _tft.setTextSize(1);
  const char* days[] = {"S", "M", "T", "W", "T", "F", "S"};
  int cellW = 32;
  int cellH = 28;
  
  int gridX = startX + (calWidth - (7 * cellW)) / 2;
  int gridY = startY + 60;
  
  uint16_t weekendColor = 0xFA27; // Soft Red
  uint16_t weekdayHeaderColor = 0x8410; // Muted Gray
  
  for (int i = 0; i < 7; i++) {
    if (i == 0 || i == 6) _tft.setTextColor(weekendColor); 
    else _tft.setTextColor(weekdayHeaderColor); 
    
    int charW = 6;
    _tft.setCursor(gridX + i * cellW + (cellW - charW) / 2, gridY);
    _tft.print(days[i]);
  }
  
  // 5. Calendar Days Grid
  gridY += 20; // Shift down for the numbers
  int daysInMonth = getDaysInMonth(time.month, time.year);
  int firstDay = getDayOfWeek(1, time.month, time.year);
  
  _tft.setTextSize(2);
  int row = 0;
  for (int d = 1; d <= daysInMonth; d++) {
    int col = (firstDay + d - 1) % 7;
    if (d > 1 && col == 0) row++;
    
    int cx = gridX + col * cellW + cellW / 2;
    int cy = gridY + row * cellH + cellH / 2;
    
    // Highlight current day with a beautiful solid circle
    if (d == time.day) {
      _tft.fillCircle(cx, cy, 13, 0xF800); // Bright Red for today
      _tft.setTextColor(0xFFFF); 
    } else {
      if (col == 0 || col == 6) _tft.setTextColor(weekendColor);
      else _tft.setTextColor(0xFFFF);
    }
    
    char dayStr[3];
    snprintf(dayStr, sizeof(dayStr), "%d", d);
    int dw = (d < 10) ? 12 : 24; // Font size 2 width is 12px per char
    
    // Y-offset -7 to perfectly center Size 2 text vertically
    _tft.setCursor(cx - dw / 2, cy - 7);
    _tft.print(dayStr);
  }
}
