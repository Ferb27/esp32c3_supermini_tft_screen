# ESP32-C3 SuperMini TFT Screen

Đồng hồ thông minh + màn hình TFT cho ESP32-C3 SuperMini, hiển thị thời gian, thời tiết, và hoạt hình với cảm biến gõ phím Bongo Cat từ PC.

---

## 📦 Linh kiện

| Linh kiện | Mô tả |
|-----------|-------|
| **ESP32-C3 SuperMini** | Vi điều khiển RISC-V 32-bit, WiFi, BLE |
| **Màn hình TFT ST7789** | 240x240 pixel, giao tiếp SPI |
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
| **BLK** | **GPIO10** | Backlight (có thể điều chỉnh độ sáng bằng PWM) |

> **Ghi chú BLK:** Nếu bạn cắm BLK thẳng vào 3V3 thì đèn nền sẽ luôn sáng max. Cắm vào **GPIO10** cho phép điều chỉnh độ sáng hoặc tắt màn hình bằng phần mềm.

---

## ⚙️ Cấu hình Arduino IDE

| Cài đặt | Giá trị |
|---------|---------|
| Board | **ESP32-C3 (SuperMini)** |
| **USB CDC on Boot** | **Enabled** |
| Upload Speed | **115200** |
| Flash Size | 4MB |
| Partition Scheme | Default 4MB |

> **USB CDC on Boot = Enabled** là bắt buộc để ESP32-C3 SuperMini giao tiếp Serial qua USB sau khi nạp code. Nếu để Disabled, cổng COM sẽ biến mất sau khi nạp.

---

## 📁 Cấu trúc thư mục

```
esp32c3_supermini_tft_screen/
├── esp32_c3_st7789_clock_ui_demo/       # Bản gốc - đồng hồ cơ bản
├── esp32_c3_st7789_clock_ui_demo_v2/    # Các bản cải tiến dần
├── ...
├── esp32_c3_st7789_clock_ui_demo_v9/
├── esp32_c3_st7789_clock_ui_demo_v10/   # + Animation, RLE nén frame, icon thời tiết
├── esp32_c3_st7789_clock_ui_demo_v11/   # Cải thiện animation
├── esp32_c3_st7789_clock_ui_demo_v12_no_anime/  # Bỏ anime, thêm Bongo Cat
├── esp32_c3_st7789_clock_ui_demo_v13/   # Phiên bản mới nhất
├── BongoClient/          # Ứng dụng Python PC - đếm phím gửi qua Serial
├── test_st7789_screen/   # Test màn hình
├── test_button__esp32_c3_supermini/     # Test nút bấm
├── icon/                 # Icon
├── mcp/                  # Pixel MCP server (Aseprite ↔ AI)
├── process_bongo.py      # Tool xử lý sprite Bongo Cat
├── process_cat.py        # Tool xử lý ảnh mèo
├── mcp.json              # Cấu hình MCP
└── *.aseprite / *.png    # File gốc sprite, frame ảnh
```

---

## 🚀 Hướng dẫn nạp code (mới nhất)

1. Mở file **`esp32_c3_st7789_clock_ui_demo_v13/esp32_c3_st7789_clock_ui_demo_v13.ino`** trong Arduino IDE
2. Cấu hình board như bảng ở trên
3. Cắm ESP32-C3 qua USB
4. Chọn đúng cổng COM
5. Nhấn **Upload**

> Nếu gặp lỗi upload, thử giữ nút **BOOT** trên board, nhấn Reset rồi thả BOOT, sau đó upload lại.

---

## 🎮 Tính năng

- **Đồng hồ** — thời gian thực qua NTP, hiển thị giờ:phút:giây
- **Thời tiết** — gọi OpenWeatherMap API, tự động refresh
- **Nhiều layout** — nhấn nút GPIO5 để chuyển giao diện
- **Bongo Cat** — kết nối với PC qua Serial, gõ phím thì mèo gõ theo
- **Animation** — chạy hoạt hình Bad Apple / GIF (bản v10, v11)

---

## 📜 Giấy phép

MIT
