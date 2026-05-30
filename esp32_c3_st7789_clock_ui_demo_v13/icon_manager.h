#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "icons.h"

enum StaticIconType {
  ICON_TYPE_TEMP,
  ICON_TYPE_HUMIDITY,
  ICON_TYPE_DECOR
};

class IconManager {
public:
  // Vẽ bitmap RGB565 có xử lý màu nền trong suốt cho Adafruit_GFX
  static void drawTransparentBitmap(Adafruit_ST7789& tft, int x, int y, const uint16_t *bitmap, int w, int h);

  // Vẽ icon thời tiết 32x32 dựa trên weather code và giờ thực tế (để hiển thị Mặt trời/Mặt trăng)
  static void drawWeatherIcon(Adafruit_ST7789& tft, int x, int y, int weatherCode, int hour = 12);

  // Vẽ các icon trạng thái tĩnh 24x24 (Nhiệt độ, độ ẩm, trang trí)
  static void drawStaticIcon(Adafruit_ST7789& tft, int x, int y, StaticIconType type, int value);
};

#endif
