#ifndef CONFIG_H
#define CONFIG_H

// Định nghĩa phần cứng
#define BTN_PIN  5

// Khoảng thời gian chống dội nút nhấn (ms)
#define DEBOUNCE_DELAY 50

// Cấu hình kết nối Wi-Fi
#define WIFI_SSID "1103"
#define WIFI_PASSWORD "12345678"

// Cấu hình OpenWeatherMap (API Key bảo mật)
#define OWM_API_KEY "d8809b4f8aba97a9eff4191a8927c8b7"

// Khoảng thời gian cập nhật thời tiết (15 phút = 900.000 ms)
#define WEATHER_UPDATE_INTERVAL 900000

#endif
