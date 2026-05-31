# ESP32-C3 SuperMini TFT Screen

Đồng hồ thông minh + màn hình TFT cho ESP32-C3 SuperMini, hiển thị thời gian, thời tiết, hoạt hình, và Bongo Cat tương tác với bàn phím PC qua Serial.

---

## 🖼 Tính năng

| Tính năng | Mô tả |
|-----------|-------|
| **Đồng hồ** | Thời gian thực qua NTP, hiển thị giờ:phút:giây, tự động múi giờ qua ip-api.com |
| **Thời tiết** | OpenWeatherMap API, tự động refresh, icon vector vẽ bằng thuật toán tọa độ |
| **Nhiều layout** | Chuyển giao diện bằng nút nhấn GPIO5: Full, Minimal, Cyberpunk, Anime |
| **Bad Apple / GIF** | Animation đằng sau mặt đồng hồ, dùng RLE nén frame tiết kiệm Flash |
| **Bongo Cat** | Kết nối PC qua Serial → gõ phím thì mèo gõ theo real-time |
| **Beat Visualizer** | (v15) Hiển thị nhịp gõ phím dạng thanh nhạc |
| **Không blocking** | Toàn bộ code dùng `millis()`, không `delay()` — mượt mà 100% |

---

## 📦 Linh kiện

| Linh kiện | Mô tả |
|-----------|-------|
| **ESP32-C3 SuperMini** | Vi điều khiển RISC-V 32-bit, WiFi, BLE |
| **Màn hình TFT ST7789** | 240×240 pixel, giao tiếp SPI |
| **Nút nhấn** | GPIO5 (chuyển layout đồng hồ) |
| **Dây nối Dupont** | Female-Female |

---

## 🔌 Sơ đồ chân (Wiring)

| TFT ST7789 | ESP32-C3 SuperMini | Ghi chú |
|-----------|-------------------|---------|
| **VCC** | **3V3** | ⚠️ **KHÔNG cắm 5V** — sẽ chết màn hình |
| **GND** | **GND** | |
| **SCL (SCK)** | **GPIO4** | SPI Clock |
| **SDA (MOSI)** | **GPIO6** | SPI Data |
| **RST** | **GPIO3** | Reset |
| **DC** | **GPIO2** | Data/Command |
| **CS** | **GPIO7** | Chip Select |
| **BLK** | **GPIO8** (v15) / **GPIO10** (v10-v14) | Backlight (có thể PWM) |

> **Về BLK:** GPIO8 (v15) hoặc GPIO10 (v10-v14) cho phép điều chỉnh độ sáng bằng PWM. Nếu muốn sáng max, cắm thẳng vào 3V3.

---

## ⚙️ Cấu hình Arduino IDE

| Cài đặt | Giá trị |
|---------|---------|
| Board | **ESP32-C3 (SuperMini)** hoặc **ESP32C3 Dev Module** |
| **USB CDC on Boot** | **Enabled** |
| Upload Speed | **115200** |
| Flash Size | 4MB |
| Partition Scheme | Default 4MB |

> **USB CDC on Boot = Enabled** là bắt buộc để ESP32-C3 SuperMini giao tiếp Serial qua USB sau khi nạp code.

---

## 🚀 Nạp code (bản mới nhất)

### Với đồng hồ + Bongo Cat (khuyên dùng)
1. Mở `esp32c3_clock_v14_bongo_cat_native_geometry/esp32c3_clock_v14_bongo_cat_native_geometry.ino`
2. Cấu hình board như bảng trên
3. Upload lên ESP32-C3
4. Chạy bên PC: `BongoClient_v12_v13_v14/BongoCatClient_v7.spec` (hoặc app.py tương ứng)

### Với đồng hồ + Beat Visualizer
1. Mở `esp32c3_clock_v15_beat_visualizer/esp32c3_clock_v15_beat_visualizer.ino`
2. Upload lên ESP32-C3
3. Chạy bên PC: `BongoClient_v15_beat_visualizer/app.py`

> Nếu lỗi upload, giữ nút **BOOT**, nhấn Reset rồi thả BOOT, upload lại.

---

## 📁 Cấu trúc thư mục

### 🔸 Firmware ESP32-C3 (đánh số theo tiến trình phát triển)

| Thư mục | Mô tả |
|---------|-------|
| `esp32c3_clock_v1_three_faces` | Bản gốc — 3 mặt đồng hồ cơ bản |
| `esp32c3_clock_v2_mock_weather` | + Mock dữ liệu thời tiết |
| `esp32c3_clock_v3_layout_tweaks` | + Tinh chỉnh layout |
| `esp32c3_clock_v4_temp_humidity_bars` | + Thanh nhiệt/độ ẩm |
| `esp32c3_clock_v5_dynamic_weather_icon` | + Icon thời tiết động |
| `esp32c3_clock_v6_basic_cat_icon` | + Icon mèo cơ bản |
| `esp32c3_clock_v7_cute_cat_icon` | + Icon mèo dễ thương hơn |
| `esp32c3_clock_v8_paw_icon` | + Icon chân mèo |
| `esp32c3_clock_v9_paw_layout_adjust` | + Chỉnh layout paw |
| `esp32c3_clock_v10_wifi_weather_bad_apple` | **WiFi thật + Animation Bad Apple** (RLE nén frame, DMA SPI) |
| `esp32c3_clock_v11_layout_preferences` | + Lưu layout preference |
| `esp32c3_clock_v12_bongo_cat_bitmap` | + Bongo Cat dạng bitmap sprites |
| `esp32c3_clock_v13_bongo_cat_png_sprites` | + Bongo Cat dùng PNG sprites |
| `esp32c3_clock_v14_bongo_cat_native_geometry` | **★ Bongo Cat Flat Design** — vẽ bằng hình khối, không tốn Flash |
| `esp32c3_clock_v15_beat_visualizer` | **★ Mới nhất** — Beat Visualizer + thanh nhạc theo nhịp gõ phím |

### 🔸 Ứng dụng PC (Python)

| Thư mục | Mô tả |
|---------|-------|
| `BongoClient_v12_v13_v14` | Python app đếm phím → Serial (dùng với v12-v14) |
| `BongoClient_v15_beat_visualizer` | Python app đếm phím → gửi cường độ nhịp (dùng với v15) |

### 🔸 Khác

| Thư mục / File | Mô tả |
|----------------|-------|
| `test_st7789_screen/` | Test màn hình |
| `test_button__esp32_c3_supermini/` | Test nút bấm |
| `icon/` | Icon gốc |
| `process_bongo.py` | Tool xử lý sprite Bongo Cat |
| `process_cat.py` | Tool xử lý ảnh mèo |
| `bongo_animated.aseprite` / `bongo_cat.aseprite` / `bongo_cat_real.aseprite` | File sprite gốc (Aseprite) |
| `frame1.png` / `frame2.png` / `frame3.png` | Frame ảnh mẫu |

---

## 🧠 Kiến trúc kỹ thuật

- **Non-blocking hoàn toàn**: Mọi logic dùng `millis()` — không `delay()`
- **RLE nén frame**: Animation Bad Apple/GIF nén 5-10x so với bitmap thô
- **Bongo Cat Native Geometry**: Vẽ mèo bằng hình khối cơ bản (flat design) — zero byte cho ảnh
- **Serial Protocol**: Giao thức `K:L\n` / `K:R\n` chống nhiễu USB, không drop phím
- **WiFi + API**: ip-api.com (múi giờ), OpenWeatherMap (thời tiết), NTP (giờ)

---

## 📜 Giấy phép

MIT
