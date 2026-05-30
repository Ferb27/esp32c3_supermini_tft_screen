#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

struct WeatherData {
  int temp;            // Nhiệt độ hiện tại (°C)
  int humidity;        // Độ ẩm (%)
  String city;         // Tên thành phố
  String country;      // Mã quốc gia (VD: "VN")
  int weatherCode;     // Mã code thời tiết (OpenWeatherMap ID)
  String lastUpdated;  // Thời gian cập nhật cuối cùng (HH:MM)
};

struct DateTimeData {
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
  String dayOfWeek;    // "Mon", "Tue", etc.
};

struct PetData {
  uint32_t todayKeystrokes;
  uint32_t history[7]; // 7 ngày gần nhất (0: hôm qua, 6: 7 ngày trước)
  uint32_t currentStreak;
  uint32_t maxStreak;
  unsigned long lastKeystrokeTime;
};

enum LayoutMode {
  LAYOUT_FULL = 0,
  LAYOUT_MINIMAL,
  LAYOUT_CYBERPUNK,
  LAYOUT_ANIME,
  LAYOUT_BONGO_CAT,
  LAYOUT_COUNT
};

struct SystemState {
  LayoutMode currentLayout;
  bool wifiConnected;
};

#endif
