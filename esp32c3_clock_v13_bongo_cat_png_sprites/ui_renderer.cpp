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
    // Fill the inner area first, avoiding corners
    _tft.fillRect(x + cutSize, y, w - 2*cutSize, h, bgColor);
    _tft.fillRect(x, y + cutSize, w, h - 2*cutSize, bgColor);
    // Fill corners
    _tft.fillTriangle(x, y+cutSize, x+cutSize, y, x+cutSize, y+cutSize, bgColor);
    _tft.fillTriangle(x+w-cutSize-1, y, x+w-1, y+cutSize, x+w-cutSize-1, y+cutSize, bgColor);
    _tft.fillTriangle(x, y+h-cutSize-1, x+cutSize, y+h-1, x+cutSize, y+h-cutSize-1, bgColor);
    _tft.fillTriangle(x+w-1, y+h-cutSize-1, x+w-cutSize-1, y+h-1, x+w-cutSize-1, y+h-cutSize-1, bgColor);
  }
  
  // Draw the border lines
  _tft.drawLine(x + cutSize, y, x + w - cutSize - 1, y, color); // top
  _tft.drawLine(x + w - 1, y + cutSize, x + w - 1, y + h - cutSize - 1, color); // right
  _tft.drawLine(x + cutSize, y + h - 1, x + w - cutSize - 1, y + h - 1, color); // bottom
  _tft.drawLine(x, y + cutSize, x, y + h - cutSize - 1, color); // left
  
  // Draw angled corners
  _tft.drawLine(x, y + cutSize, x + cutSize, y, color); // top left
  _tft.drawLine(x + w - cutSize - 1, y, x + w - 1, y + cutSize, color); // top right
  _tft.drawLine(x, y + h - cutSize - 1, x + cutSize, y + h - 1, color); // bottom left
  _tft.drawLine(x + w - 1, y + h - cutSize - 1, x + w - cutSize - 1, y + h - 1, color); // bottom right
}

void UIRenderer::drawStaticLayout(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, const PetData* pet) {
  _tft.fillScreen(0x0000);
  
  if (mode == LAYOUT_FULL) {
    _tft.setTextSize(2);
    // Tính toán căn giữa: city + khoảng cách + badge
    int cityPxW = weather.city.length() * 12;
    int badgeW = weather.country.length() * 12 + 8;
    int totalW = cityPxW + 6 + badgeW; // 6px gap
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
    // 1. Đường phân cách dọc mỏng thanh lịch màu xám xanh
    _tft.drawFastVLine(115, 20, 200, 0x31A6);
    
    // 2. Icon thời tiết 40x40 ở góc phải trên (căn giữa cột bên phải)
    IconManager::drawWeatherIcon(_tft, 155, 30, weather.weatherCode, time.hour);
  }
  else if (mode == LAYOUT_CYBERPUNK) {
    // 1. Grid background dense
    for (int i = 0; i < 240; i += 15) {
      _tft.drawFastHLine(0, i, 240, 0x0841); 
      _tft.drawFastVLine(i, 0, 240, 0x0841);
    }
    
    // Main central cut-corner frame
    drawCyberFrame(0, 0, 240, 240, 20, 0x07FF, 0x0000, false);
    
    // Top banner
    drawCyberFrame(15, -5, 210, 34, 8, 0x07FF, 0x07FF, true);
    _tft.setTextColor(0x0000); 
    _tft.setTextSize(2);
    _tft.setCursor(55, 8);
    _tft.print("NEURAL LINK");

    // Decorative static data
    _tft.setTextColor(0xF81F);
    _tft.setTextSize(1);
    _tft.setCursor(5, 31);
    _tft.print("SEC_09");
    _tft.setCursor(195, 31);
    _tft.print("OVERRIDE");

    // Vertical side texts
    _tft.setTextColor(0x07FF);
    for (int i=0; i<6; i++) {
      _tft.setCursor(5, 60 + i*15);
      _tft.print(i % 2 == 0 ? "1" : "0");
      _tft.setCursor(230, 60 + i*15);
      _tft.print(i % 2 == 0 ? "0" : "1");
    }
    
    // Crosshair target
    _tft.drawLine(120, 35, 120, 65, 0xF81F);
    _tft.drawLine(115, 50, 125, 50, 0xF81F);

    // Bottom tech panels for Temp/Humid
    drawCyberFrame(8, 178, 105, 54, 8, 0x07FF, 0x2104, true);
    _tft.setTextColor(0x07FF);
    _tft.setTextSize(2);
    _tft.setCursor(19, 184);
    _tft.print("ENV.TMP");
    
    drawCyberFrame(127, 178, 105, 54, 8, 0x07FF, 0x2104, true);
    _tft.setCursor(138, 184);
    _tft.print("ENV.HUM");
    
    // Bottom center triangle
    _tft.fillTriangle(120, 178, 115, 188, 125, 188, 0xF81F);
    _tft.fillTriangle(120, 232, 115, 222, 125, 222, 0xF81F);
  }
  else if (mode == LAYOUT_BONGO_CAT) {
    // Vẽ giao diện tĩnh cho Bongo Cat
    _tft.fillScreen(0x0000); // Nền đen
    
    // Đường phân cách cho đồng hồ nhỏ
    _tft.drawRoundRect(10, 10, 80, 30, 5, 0x07FF);
    
    // Vẽ Bongo cat tĩnh ban đầu (Idle)
    updateBongoAnimation(0);
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
      // Giờ phút (Chỉ vẽ lại khi đổi phút hoặc force)
      if (force || time.minute != _prevTime.minute || time.hour != _prevTime.hour) {
        _tft.fillRect(15, 55, 85, 120, 0x0000);
        
        // Giờ (Trắng)
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(6);
        _tft.setCursor(21, 60);
        if (time.hour < 10) _tft.print('0');
        _tft.print(time.hour);
        
        // Phút (Vàng)
        _tft.setTextColor(0xFEE0);
        _tft.setCursor(21, 120);
        if (time.minute < 10) _tft.print('0');
        _tft.print(time.minute);
      }
      
      // Nhiệt độ (Chỉ vẽ lại khi đổi nhiệt độ hoặc force)
      if (force || weather.temp != _prevWeather.temp) {
        _tft.fillRect(130, 90, 95, 26, 0x0000);
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(3);
        _tft.setCursor(148, 90);
        _tft.print(weather.temp); _tft.print((char)247); // Ký tự độ °
      }
      
      // Thứ & Ngày/Tháng (Chỉ vẽ lại khi đổi ngày hoặc force)
      if (force || time.day != _prevTime.day) {
        _tft.fillRect(130, 140, 95, 50, 0x0000);
        
        // Thứ (Cyan)
        _tft.setTextColor(0x07FF);
        _tft.setTextSize(2);
        _tft.setCursor(157, 140);
        _tft.print(time.dayOfWeek);
        
        // Ngày/Tháng (Trắng)
        _tft.setTextColor(0xFFFF);
        _tft.setCursor(145, 165);
        _tft.printf("%02d/%02d", time.day, time.month);
      }
      
      // Độ ẩm (Chỉ vẽ lại khi đổi độ ẩm hoặc force)
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
      
      // LOCATION DATA (centered)
      _tft.fillRect(0, 42, 240, 16, 0x0000); // Clear area
      _tft.setTextSize(2);
      // Tính chiều dài tổng cộng để căn giữa
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

      // DYNAMIC BAR CHART
      _tft.fillRect(180, 145, 40, 20, 0x0000);
      for (int i = 0; i < 6; i++) {
        int h = random(2, 18);
        _tft.fillRect(180 + i*6, 165 - h, 4, h, 0x07FF);
      }
      
      // BLINKING SYSTEM STATUS
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
      drawCyberDigit(_clockCanvas, 15, y, h_t, 0xFE60); // Yellow
      drawCyberDigit(_clockCanvas, 51, y, h_u, 0xFE60);
      
      // Blinking colon & REC warning
      if (time.second % 2 == 0) {
        _clockCanvas->fillRect(92, y+10, 6, 6, 0xF81F);
        _clockCanvas->fillRect(92, y+32, 6, 6, 0xF81F);
        
        _clockCanvas->setTextColor(0xF800); // Red
        _clockCanvas->setTextSize(1);
        _clockCanvas->setCursor(0, 0);
        _clockCanvas->print("[REC]");
      }
      
      drawCyberDigit(_clockCanvas, 111, y, m_t, 0xFE60);
      drawCyberDigit(_clockCanvas, 147, y, m_u, 0xFE60);
      
      // Seconds using Custom Cyber Font (Smaller)
      int s_t = time.second / 10;
      int s_u = time.second % 10;
      drawCyberDigit(_clockCanvas, 182, y+24, s_t, 0x07FF, 14, 24, 3);
      drawCyberDigit(_clockCanvas, 200, y+24, s_u, 0x07FF, 14, 24, 3);
      
      // Segmented Progress Bar
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
      // Cập nhật đồng hồ nhỏ góc trái
      _tft.fillRect(12, 12, 76, 26, 0x0000);
      _tft.setTextColor(0xFFFF);
      _tft.setTextSize(2);
      _tft.setCursor(15, 17);
      if (time.hour < 10) _tft.print('0');
      _tft.print(time.hour);
      if (time.second % 2 == 0) _tft.print(':'); else _tft.print(' ');
      if (time.minute < 10) _tft.print('0');
      _tft.print(time.minute);
      
      // Vẽ Text báo số phím gõ hôm nay
      if (pet != nullptr) {
        _tft.fillRect(95, 10, 135, 30, 0x0000);
        _tft.setTextColor(0xFE60); // Vàng
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
      _tft.setTextColor(0xFE60, 0x2104); // Yellow text on dark background
      _tft.setTextSize(3);
      _tft.setCursor(30, 203);
      _tft.print(weather.temp); 
      _tft.setTextSize(2);
      _tft.print("C");
    }
    if (force || weather.humidity != _prevWeather.humidity) {
      _tft.fillRect(131, 201, 97, 26, 0x2104);
      _tft.setTextColor(0x07FF, 0x2104); // Cyan text on dark background
      _tft.setTextSize(3);
      _tft.setCursor(155, 203);
      _tft.print(weather.humidity);
      _tft.setTextSize(2);
      _tft.print("%");
    }
  }
  else if (mode == LAYOUT_BONGO_CAT && pet != nullptr) {
    if (force) {
      _tft.fillRect(0, 150, 240, 90, 0x0000); // Clear vùng bên dưới
      if (pet->currentStreak > 1) {
        // Màu sắc thay đổi theo độ lớn của streak
        uint16_t streakColor = 0xF800; // Đỏ
        if (pet->currentStreak > 30) streakColor = 0xFE60; // Vàng (Lửa)
        if (pet->currentStreak > 80) streakColor = 0x07FF; // Cyan (Băng)
        if (pet->currentStreak > 150) streakColor = 0xF81F; // Hồng
        
        // Rung lắc nhẹ (shaking effect)
        int offsetX = (pet->currentStreak > 15) ? random(-3, 4) : 0;
        int offsetY = (pet->currentStreak > 15) ? random(-3, 4) : 0;
        
        _tft.setTextColor(streakColor);
        _tft.setTextSize(3);
        
        // Căn giữa text "COMBO xNN"
        String comboText = "COMBO x" + String(pet->currentStreak);
        int textWidth = comboText.length() * 18; // Kích thước mỗi ký tự size 3 là ~18px
        int textX = (240 - textWidth) / 2 + offsetX;
        if (textX < 0) textX = 0;
        
        _tft.setCursor(textX, 170 + offsetY);
        _tft.print(comboText);
        
        // Vẽ dòng chữ Max Streak nhỏ bên dưới
        _tft.setTextColor(0x07E0); // Xanh lá
        _tft.setTextSize(1);
        String maxText = "MAX COMBO: " + String(pet->maxStreak);
        int maxW = maxText.length() * 6;
        _tft.setCursor((240 - maxW)/2, 210);
        _tft.print(maxText);
      } else {
        // Khi không có streak, hiển thị Max Streak đẹp đẽ
        _tft.setTextColor(0x7BEF); // Xám xám
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

// Helper: vẽ đường thẳng siêu dày bo tròn 2 đầu (Rất phù hợp để vẽ tay mèo)
static void drawThickLine(Adafruit_ST7789& tft, int x0, int y0, int x1, int y1, int r, uint16_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;
  while (true) {
    tft.fillCircle(x0, y0, r, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void UIRenderer::updateBongoAnimation(int frameState) {
  if (frameState < 0 || frameState > 3) frameState = 0;
  
  // Xóa nguyên khu vực bằng nền đen
  _tft.fillRect(0, 42, 240, 105, 0x0000);
  
  int catX = 120;
  int catY = 85; 
  int tableY = 125;
  
  // === 1. CƠ THỂ MÈO (Phía sau bàn) ===
  // Viền đen thân (Đầu tròn dome, thân phẳng)
  _tft.fillCircle(catX, catY, 46, 0x0000);
  _tft.fillRect(catX - 46, catY, 92, tableY - catY, 0x0000);
  
  // Viền đen tai (Vẽ lùi ra sau, chếch ra ngoài)
  _tft.fillTriangle(catX - 42, catY - 15, catX - 25, catY - 48, catX - 8, catY - 35, 0x0000);
  _tft.fillTriangle(catX + 42, catY - 15, catX + 25, catY - 48, catX + 8, catY - 35, 0x0000);
  
  // Phủ màu trắng thân
  _tft.fillCircle(catX, catY, 43, 0xFFFF);
  _tft.fillRect(catX - 43, catY, 86, tableY - catY, 0xFFFF);
  
  // Phủ màu trắng tai
  _tft.fillTriangle(catX - 38, catY - 17, catX - 25, catY - 44, catX - 11, catY - 34, 0xFFFF);
  _tft.fillTriangle(catX + 38, catY - 17, catX + 25, catY - 44, catX + 11, catY - 34, 0xFFFF);
  
  // Khuôn mặt ngốc nghếch đặc trưng Bongo (Mắt xa nhau)
  _tft.fillCircle(catX - 24, catY + 2, 4, 0x0000);
  _tft.fillCircle(catX + 24, catY + 2, 4, 0x0000);
  
  // Miệng chữ W nhỏ nhắn, cute
  _tft.drawLine(catX - 6, catY + 6, catX - 3, catY + 10, 0x0000);
  _tft.drawLine(catX - 3, catY + 10, catX, catY + 6, 0x0000);
  _tft.drawLine(catX, catY + 6, catX + 3, catY + 10, 0x0000);
  _tft.drawLine(catX + 3, catY + 10, catX + 6, catY + 6, 0x0000);
  // Dày thêm 1 pixel
  _tft.drawLine(catX - 6, catY + 7, catX - 3, catY + 11, 0x0000);
  _tft.drawLine(catX - 3, catY + 11, catX, catY + 7, 0x0000);
  _tft.drawLine(catX, catY + 7, catX + 3, catY + 11, 0x0000);
  _tft.drawLine(catX + 3, catY + 11, catX + 6, catY + 7, 0x0000);

  // === 2. MẶT BÀN & BÀN PHÍM (Che đi nửa dưới của mèo) ===
  _tft.fillRect(0, tableY, 240, 147 - tableY, 0x2124); 
  _tft.drawLine(0, tableY, 240, tableY, 0xFFFF); 
  
  int kbX = 30, kbY = tableY - 12, kbW = 180, kbH = 32;
  // Khung bàn phím bo viền
  _tft.fillRoundRect(kbX, kbY, kbW, kbH, 6, 0x0000); 
  _tft.fillRoundRect(kbX+2, kbY+2, kbW-4, kbH-4, 4, 0xBDD7); // Xám ánh xanh dương xịn xò
  
  // Chi tiết các phím bấm vuông vức
  int keyW = 12, keyH = 10;
  for(int row=0; row<2; row++) {
    for(int col=0; col<10; col++) {
      int kx = kbX + 10 + col * (keyW + 3) + (row * 6);
      int ky = kbY + 4 + row * (keyH + 4);
      _tft.fillRect(kx, ky, keyW, keyH, 0x0000); 
      _tft.fillRect(kx+1, ky+1, keyW-2, keyH-2, 0xFFFF); 
    }
  }

  // === 3. BÀN TAY (Nhỏ gọn, tròn, chỉ di chuyển lên/xuống) ===
  bool leftDown  = (frameState == 1 || frameState == 3);
  bool rightDown = (frameState == 2 || frameState == 3);
  uint16_t pinkColor = 0xFBEF;
  int pawR = 8; // Bán kính bàn tay nhỏ gọn
  
  // -- TAY TRÁI --
  int lpX = catX - 48; // Vị trí ngang: bên trái thân mèo
  if (leftDown) {
    // Đập xuống: tay úp trên bàn phím
    int lpY = kbY + 5;
    _tft.fillCircle(lpX, lpY, pawR + 2, 0x0000);
    _tft.fillCircle(lpX, lpY, pawR, 0xFFFF);
  } else {
    // Giơ lên: tay ngửa, lộ nệm thịt hồng
    int lpY = tableY - 18;
    _tft.fillCircle(lpX, lpY, pawR + 2, 0x0000);
    _tft.fillCircle(lpX, lpY, pawR, 0xFFFF);
    _tft.fillCircle(lpX, lpY + 1, 4, pinkColor);
    _tft.fillCircle(lpX - 4, lpY - 4, 2, pinkColor);
    _tft.fillCircle(lpX,     lpY - 5, 2, pinkColor);
    _tft.fillCircle(lpX + 4, lpY - 4, 2, pinkColor);
  }
  
  // -- TAY PHẢI --
  int rpX = catX + 48;
  if (rightDown) {
    int rpY = kbY + 5;
    _tft.fillCircle(rpX, rpY, pawR + 2, 0x0000);
    _tft.fillCircle(rpX, rpY, pawR, 0xFFFF);
  } else {
    int rpY = tableY - 18;
    _tft.fillCircle(rpX, rpY, pawR + 2, 0x0000);
    _tft.fillCircle(rpX, rpY, pawR, 0xFFFF);
    _tft.fillCircle(rpX, rpY + 1, 4, pinkColor);
    _tft.fillCircle(rpX - 4, rpY - 4, 2, pinkColor);
    _tft.fillCircle(rpX,     rpY - 5, 2, pinkColor);
    _tft.fillCircle(rpX + 4, rpY - 4, 2, pinkColor);
  }
}
