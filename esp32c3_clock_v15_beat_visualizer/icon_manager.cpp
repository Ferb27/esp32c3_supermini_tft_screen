#include "icon_manager.h"

void IconManager::drawTransparentBitmap(Adafruit_ST7789& tft, int x, int y, const uint16_t *bitmap, int w, int h) {
  tft.startWrite();
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      uint16_t color = pgm_read_word(bitmap + (j * w + i));
      if (color != TRANSPARENT_COLOR) {
        tft.writePixel(x + i, y + j, color);
      }
    }
  }
  tft.endWrite();
}

bool isBitmapEmpty(const uint16_t* bitmap, int size) {
  for (int i=0; i<16; i++) {
    if (pgm_read_word(bitmap + i) != 0x0000) return false;
  }
  return true;
}

void drawVectorCloud(Adafruit_ST7789& tft, int x, int y, uint16_t color) {
  tft.fillCircle(x + 13, y + 24, 8, color);
  tft.fillCircle(x + 27, y + 24, 8, color);
  tft.fillCircle(x + 20, y + 17, 11, color);
  tft.fillRect(x + 13, y + 18, 14, 15, color);
}

void IconManager::drawWeatherIcon(Adafruit_ST7789& tft, int x, int y, int weatherCode, int hour) {
  const uint16_t* selectedIcon = icon_weather_sun; 

  if (weatherCode >= 200 && weatherCode < 300) {
    selectedIcon = icon_weather_storm; 
  } else if (weatherCode >= 300 && weatherCode < 600) {
    selectedIcon = icon_weather_rain;  
  } else if (weatherCode >= 700 && weatherCode < 800) {
    selectedIcon = icon_weather_cloud;
  } else if (weatherCode == 800) {
    selectedIcon = icon_weather_sun;   
  } else if (weatherCode > 800) {
    selectedIcon = icon_weather_cloud; 
  }

  tft.fillRect(x, y, ICON_SIZE_WEATHER, ICON_SIZE_WEATHER, 0x0000);
  
  if (isBitmapEmpty(selectedIcon, 1024)) {
    bool isNight = (hour >= 18 || hour < 6);
    
    if (weatherCode >= 200 && weatherCode < 300) {
      uint16_t cloudColor = 0x5AEB;
      drawVectorCloud(tft, x, y - 2, cloudColor);
      uint16_t boltColor = 0xFE60;
      tft.drawLine(x + 20, y + 24, x + 16, y + 31, boltColor);
      tft.drawLine(x + 21, y + 24, x + 17, y + 31, boltColor);
      tft.drawLine(x + 16, y + 31, x + 23, y + 31, boltColor);
      tft.drawLine(x + 16, y + 32, x + 23, y + 32, boltColor);
      tft.drawLine(x + 23, y + 31, x + 19, y + 38, boltColor);
      tft.drawLine(x + 22, y + 31, x + 18, y + 38, boltColor);
    } 
    else if (weatherCode >= 300 && weatherCode < 600) {
      uint16_t cloudColor = 0x7BEF;
      drawVectorCloud(tft, x, y - 2, cloudColor);
      uint16_t rainColor = 0x07FF;
      tft.drawLine(x + 14, y + 26, x + 11, y + 33, rainColor);
      tft.drawLine(x + 20, y + 26, x + 17, y + 33, rainColor);
      tft.drawLine(x + 26, y + 26, x + 23, y + 33, rainColor);
      tft.drawLine(x + 15, y + 26, x + 12, y + 33, rainColor);
      tft.drawLine(x + 21, y + 26, x + 18, y + 33, rainColor);
      tft.drawLine(x + 27, y + 26, x + 24, y + 33, rainColor);
    } 
    else if ((weatherCode >= 700 && weatherCode < 800) || weatherCode > 800) {
      if (isNight) {
        uint16_t moonColor = 0xFFE0;
        tft.fillCircle(x + 22, y + 17, 14, moonColor);
        tft.fillCircle(x + 14, y + 12, 14, 0x0000);
        uint16_t cloudColor = 0x7BEF;
        tft.fillCircle(x + 9,  y + 30, 5, cloudColor);
        tft.fillCircle(x + 19, y + 30, 5, cloudColor);
        tft.fillCircle(x + 14, y + 25, 7, cloudColor);
        tft.fillRect(x + 9, y + 25, 10, 10, cloudColor);
        tft.drawFastHLine(x + 9, y + 35, 10, 0x5AEB);
        uint16_t starColor = 0xFEE0;
        tft.drawPixel(x + 5, y + 16, 0xFFFF);
        tft.drawPixel(x + 5, y + 15, starColor);
        tft.drawPixel(x + 5, y + 17, starColor);
        tft.drawPixel(x + 4, y + 16, starColor);
        tft.drawPixel(x + 6, y + 16, starColor);
        tft.drawPixel(x + 35, y + 10, 0xFFFF);
        tft.drawPixel(x + 35, y + 9,  starColor);
        tft.drawPixel(x + 35, y + 11, starColor);
        tft.drawPixel(x + 34, y + 10, starColor);
        tft.drawPixel(x + 36, y + 10, starColor);
        tft.drawPixel(x + 35, y + 22, 0xFFFF);
        tft.drawPixel(x + 35, y + 21, starColor);
        tft.drawPixel(x + 35, y + 23, starColor);
        tft.drawPixel(x + 34, y + 22, starColor);
        tft.drawPixel(x + 36, y + 22, starColor);
        tft.drawPixel(x + 11, y + 5, 0xFFFF);
        tft.drawPixel(x + 11, y + 4, starColor);
        tft.drawPixel(x + 11, y + 6, starColor);
        tft.drawPixel(x + 10, y + 5, starColor);
        tft.drawPixel(x + 12, y + 5, starColor);
      } else {
        uint16_t sunColor = 0xFE60;
        tft.fillCircle(x + 28, y + 13, 7, sunColor);
        tft.fillCircle(x + 28, y + 13, 3, 0xFFFF);
        tft.drawLine(x + 28, y + 4,  x + 28, y + 2,  sunColor);
        tft.drawLine(x + 37, y + 13, x + 39, y + 13, sunColor);
        tft.drawLine(x + 34, y + 7,  x + 36, y + 5,  sunColor);
        uint16_t cloudColor = 0xFFFF;
        tft.fillCircle(x + 13, y + 25, 6, cloudColor);
        tft.fillCircle(x + 25, y + 25, 6, cloudColor);
        tft.fillCircle(x + 19, y + 20, 8, cloudColor);
        tft.fillRect(x + 13, y + 20, 12, 11, cloudColor);
        tft.drawFastHLine(x + 13, y + 31, 12, 0xCED8);
      }
    } 
    else {
      if (isNight) {
        uint16_t moonColor = 0xFEE0;
        tft.fillCircle(x + 20, y + 20, 11, moonColor);
        tft.fillCircle(x + 15, y + 17, 11, 0x0000);
        tft.drawPixel(x + 9,  y + 10, 0xFFFF);
        tft.drawPixel(x + 30, y + 8,  0xFFFF);
        tft.drawPixel(x + 11, y + 30, 0xFFFF);
        tft.drawPixel(x + 32, y + 28, 0xFFFF);
      } else {
        uint16_t sunColor = 0xFE60;
        tft.fillCircle(x + 20, y + 20, 9, sunColor);
        tft.fillCircle(x + 20, y + 20, 4, 0xFFFF);
        for (int i = 0; i < 8; i++) {
          float a = i * 0.785398f;
          tft.drawLine(x + 20 + cos(a)*12, y + 20 + sin(a)*12, x + 20 + cos(a)*17, y + 20 + sin(a)*17, sunColor);
        }
      }
    }
  } else {
    drawTransparentBitmap(tft, x + 4, y + 4, selectedIcon, 32, 32);
  }
}

void IconManager::drawStaticIcon(Adafruit_ST7789& tft, int x, int y, StaticIconType type, int value) {
  const uint16_t* selectedIcon = nullptr;

  switch (type) {
    case ICON_TYPE_TEMP:
      selectedIcon = (value < 22) ? icon_temp_cold : ((value < 32) ? icon_temp_normal : icon_temp_hot);
      break;
    case ICON_TYPE_HUMIDITY:
      selectedIcon = icon_humidity;
      break;
    case ICON_TYPE_DECOR:
      selectedIcon = icon_cat_paw;
      break;
  }

  if (selectedIcon != nullptr) {
    tft.fillRect(x, y, ICON_SIZE_STATUS, ICON_SIZE_STATUS, 0x0000);
    
    if (isBitmapEmpty(selectedIcon, 1024)) {
      if (type == ICON_TYPE_TEMP) {
        if (value < 22) {
          uint16_t coldColor = 0x07FF;
          tft.drawRoundRect(x + 10, y + 2, 8, 22, 4, coldColor);
          tft.drawCircle(x + 14, y + 24, 7, coldColor);
          tft.fillCircle(x + 14, y + 24, 5, coldColor);
          tft.fillRect(x + 12, y + 16, 4, 4, coldColor);
          tft.drawFastHLine(x + 21, y + 12, 7, coldColor);
          tft.drawFastVLine(x + 24, y + 9, 7, coldColor);
          tft.drawLine(x + 22, y + 10, x + 26, y + 14, coldColor);
          tft.drawLine(x + 22, y + 14, x + 26, y + 10, coldColor);
        }
        else if (value >= 32) {
          uint16_t hotColor = 0xF800;
          tft.drawRoundRect(x + 10, y + 2, 8, 22, 4, 0xFFFF);
          tft.drawCircle(x + 14, y + 24, 7, 0xFFFF);
          tft.fillCircle(x + 14, y + 24, 5, hotColor);
          tft.fillRect(x + 12, y + 6, 4, 14, hotColor);
          tft.fillTriangle(x + 21, y + 20, x + 27, y + 20, x + 24, y + 6, 0xF800);
          tft.fillTriangle(x + 22, y + 20, x + 26, y + 20, x + 24, y + 11, 0xFE60);
        }
        else {
          uint16_t fluidColor = 0xF800;
          tft.drawRoundRect(x + 10, y + 2, 8, 22, 4, 0xFFFF);
          tft.drawCircle(x + 14, y + 24, 7, 0xFFFF);
          tft.fillCircle(x + 14, y + 24, 5, fluidColor);
          tft.fillRect(x + 12, y + 11, 4, 9, fluidColor);
        }
      } else if (type == ICON_TYPE_HUMIDITY) {
        tft.drawCircle(x + 14, y + 20, 8, 0x07FF);
        tft.fillCircle(x + 14, y + 20, 6, 0x07FF);
        tft.fillTriangle(x + 14, y + 4, x + 7, y + 16, x + 21, y + 16, 0x07FF);
      } else if (type == ICON_TYPE_DECOR) {
        tft.fillCircle(x + 16, y + 24, 8, 0xFD20);
        tft.fillCircle(x + 8,  y + 12, 4, 0xFD20);
        tft.fillCircle(x + 14, y + 8,  4, 0xFD20);
        tft.fillCircle(x + 22, y + 8,  4, 0xFD20);
        tft.fillCircle(x + 28, y + 14, 4, 0xFD20);
      }
    } else {
      drawTransparentBitmap(tft, x, y, selectedIcon, ICON_SIZE_STATUS, ICON_SIZE_STATUS);
    }
  }
}
