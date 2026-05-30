#include "ui_renderer.h"
#include "icon_manager.h"
#include "config.h"
#include "anime_frames.h"

UIRenderer::UIRenderer(Adafruit_ST7789& tft) : _tft(tft) {
  _clockCanvas = new GFXcanvas16(220, 60);
  _prevTime = {-1, -1, -1, -1, -1, -1, ""};
  _prevWeather = {-999, -1, "", "", -1, ""};
  _currentAnimFrame = 0;
  _lastAnimUpdate = 0;
}

UIRenderer::~UIRenderer() {
  delete _clockCanvas;
  if (_animeBuffer) {
    free(_animeBuffer);
  }
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

void UIRenderer::drawStaticLayout(const WeatherData& weather, const DateTimeData& time, LayoutMode mode) {
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
  else if (mode == LAYOUT_ANIME) {
    // Fill the background
    _tft.fillScreen(0xFFFF); // White background for Bad Apple 1-bit style
    
    // Draw initial frame to prevent flash
    if (ANIME_FRAME_COUNT > 0) {
      _tft.drawBitmap((240 - ANIME_FRAME_WIDTH) / 2, (240 - ANIME_FRAME_HEIGHT) / 2, 
                      anime_frames[0], ANIME_FRAME_WIDTH, ANIME_FRAME_HEIGHT, 0x0000, 0xFFFF);
    }
  }
}

void UIRenderer::updateDynamicUI(const WeatherData& weather, const DateTimeData& time, LayoutMode mode, bool force) {
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
    else if (mode == LAYOUT_ANIME) {
      // Clock on top of the animation
      // We will clear the clock area first or use canvas to draw solid background
      _clockCanvas->fillScreen(0x0000); 
      
      _clockCanvas->setTextColor(0xFFFF); // White text
      _clockCanvas->setTextSize(5);
      
      _clockCanvas->setCursor(35, 10);
      if (time.hour < 10) _clockCanvas->print('0');
      _clockCanvas->print(time.hour);
      
      if (time.second % 2 == 0) _clockCanvas->setTextColor(0xF800); // Red colon
      else _clockCanvas->setTextColor(0x0000);
      _clockCanvas->print(':');
      
      _clockCanvas->setTextColor(0xFFFF);
      if (time.minute < 10) _clockCanvas->print('0');
      _clockCanvas->print(time.minute);
      
      // Draw a black rounded rectangle for contrast before drawing text
      // We'll draw the canvas to the top of the screen
      // Since background is white, we draw a black bounding box
      _tft.fillRoundRect(10, 10, 220, 50, 10, 0x0000);
      _tft.drawRGBBitmap(10, 10, _clockCanvas->getBuffer(), 220, 60);
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
  
  _prevTime = time;
  _prevWeather = weather;
}

void UIRenderer::updateAnimation() {
  if (ANIME_FRAME_COUNT > 0) {
    _currentAnimFrame++;
    if (_currentAnimFrame >= ANIME_FRAME_COUNT) {
      _currentAnimFrame = 0;
    }
    
    // Allocate buffer for 16-bit RGB565 frame
    if (_animeBuffer == nullptr) {
      _animeBuffer = (uint16_t*)malloc(ANIME_FRAME_WIDTH * ANIME_FRAME_HEIGHT * 2);
    }
    
    if (_animeBuffer) {
      // Decode RLE compressed PROGMEM array to 16-bit RGB565 buffer
      const unsigned char* frameData = anime_frames[_currentAnimFrame];
      bool isBlack = true;
      int pixelIndex = 0;
      int dataIndex = 0;
      int totalPixels = ANIME_FRAME_WIDTH * ANIME_FRAME_HEIGHT;
      
      while (pixelIndex < totalPixels) {
        uint8_t count = pgm_read_byte(&frameData[dataIndex++]);
        uint16_t color = isBlack ? 0x0000 : 0xFFFF;
        
        for (int c = 0; c < count; c++) {
          if (pixelIndex < totalPixels) {
            _animeBuffer[pixelIndex++] = color;
          }
        }
        isBlack = !isBlack;
      }
      
      // Push via fast DMA/SPI block
      _tft.drawRGBBitmap((240 - ANIME_FRAME_WIDTH) / 2, (240 - ANIME_FRAME_HEIGHT) / 2 + 30, 
                         _animeBuffer, ANIME_FRAME_WIDTH, ANIME_FRAME_HEIGHT);
    }
  }
}
