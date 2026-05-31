#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>

// Màu nền trong suốt của icon
#define TRANSPARENT_COLOR 0x0000

#define ICON_SIZE_WEATHER 40
#define ICON_SIZE_STATUS  32

// --- KHAI BÁO EXTERNAL CÁC MẢNG BITMAP ---
// Nội dung mảng được định nghĩa trong icons.cpp
// Hãy dùng script Python của bạn để thay thế các mảng trong icons.cpp bằng ảnh thật

extern const uint16_t icon_weather_sun[1024];
extern const uint16_t icon_weather_cloud[1024];
extern const uint16_t icon_weather_rain[1024];
extern const uint16_t icon_weather_storm[1024];

extern const uint16_t icon_temp_cold[1024];
extern const uint16_t icon_temp_normal[1024];
extern const uint16_t icon_temp_hot[1024];
extern const uint16_t icon_humidity[1024];
extern const uint16_t icon_cat_paw[1024];

#endif
