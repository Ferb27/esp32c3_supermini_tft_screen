# Bối cảnh & Hướng dẫn cho AI (Project Context & AI Instructions)

Dự án: **ESP32-C3 SuperMini TFT Screen - Clock UI Demo v10**
Mục đích của file này: Giúp các AI ở phiên làm việc (session) tiếp theo hiểu ngay lập tức kiến trúc, thuật toán và luồng chạy của code hiện tại để tiếp tục phát triển mà không phá vỡ logic cũ.

---

## 1. Nền tảng & Thư viện (Hardware & Libraries)
* **Vi điều khiển**: ESP32-C3 SuperMini.
* **Màn hình**: TFT ST7789 (240x240, giao tiếp SPI).
* **Thư viện cốt lõi**: `Adafruit_GFX` và `Adafruit_ST7789`. Vẽ các hình vector (cung tròn, nét đứt, line) và bitmap qua SPI.
* **Internet**: Sử dụng `WiFi.h` và `HTTPClient.h` để gọi các API lấy dữ liệu.

## 2. Các luồng xử lý chính (Core Architecture)
* **Tuyệt đối Non-blocking**: Toàn bộ hệ thống được thiết kế chạy bằng `millis()`. KHÔNG SỬ DỤNG `delay()` trong vòng lặp `loop()`. Việc này đảm bảo nút bấm (Ngắt - Interrupt) và tiến trình vẽ đồ họa mượt mà.
* **Dữ liệu Thời tiết & Giờ**: 
  * Định vị & Offset giờ được lấy 1 lần qua `ip-api.com`.
  * Giờ NTP được đồng bộ 1 lần duy nhất lúc khởi động, sau đó được tự đếm bộ đếm nội bộ qua hàm `updateTimeFromMillis()` siêu nhẹ.
  * Thời tiết gọi qua OpenWeatherMap API và tự động refresh định kỳ theo cấu hình (ví dụ 15 phút/lần).

## 3. Hệ thống Giao diện (UI Layouts)
Quản lý trong `ui_renderer.cpp` bằng biến `sysState.currentLayout`. Nút bấm vật lý (gắn ngắt ngoại vi) dùng để xoay vòng các mặt:
1. `LAYOUT_FULL`: Mặt đồng hồ đầy đủ chi tiết, icon thời tiết vector vẽ bằng thuật toán tọa độ tự code trong `IconManager`.
2. `LAYOUT_MINIMAL`: Tối giản hóa.
3. `LAYOUT_CYBERPUNK`: Vẽ giao diện dạng UI khoa học viễn tưởng với đường kẻ grid, progress bar.
4. `LAYOUT_ANIME`: Vẽ hoạt hình Bad Apple / GIF đằng sau mặt đồng hồ.

*(Ghi chú: Layout Test và Debug đã được xóa để tiết kiệm dung lượng Flash, hãy lưu ý không gọi lại các biến này trong `types.h`)*.

## 4. Xử lý Ảnh động (Thuật toán RLE & Frame Rendering)
Đây là phần cốt lõi và phức tạp nhất cần AI lưu ý:
* **Tool nén (`converter.html`)**: Trình duyệt xử lý file GIF (giảm độ phân giải về 120x120 Trắng/Đen), và ép xung mảng bit bằng thuật toán **RLE (Run-Length Encoding)** luân phiên Đen/Trắng. Mảng bit xuất ra file `anime_frames.h` dung lượng siêu nhẹ (giảm 5-10x so với mảng điểm ảnh thô).
* **Render thực tế (`ui_renderer.cpp`)**: Ở hàm `UIRenderer::updateAnimation()`, AI sẽ dùng một biến `uint16_t* _animeBuffer` (bộ đệm RAM) để giải mã ngược (decompress) mảng RLE theo thời gian thực (Real-time).
* **SPI DMA Block Transfer**: Sau khi giải mã mảng thành RGB565 (0x0000 và 0xFFFF), code sử dụng `_tft.drawRGBBitmap` để ép (push) 14.400 pixel ra màn hình LCD **trong một lệnh SPI duy nhất**. Tuyệt đối không dùng `drawPixel` hay hàm `drawBitmap` dạng 1-bit cũ vì nó sẽ gây giật chớp (Flicker).

---

## 5. CHỈ ĐỊNH CHO AI (AI INSTRUCTIONS FOR FUTURE SESSIONS)
Khi bạn (AI) nhận được yêu cầu thêm tính năng từ User, HÃY TUÂN THỦ CÁC QUY TẮC SAU:
1. **Bảo tồn Non-blocking**: Không được thêm hàm `delay()` vào `loop()`. Nếu cần tạo hiệu ứng chờ, hãy dùng cơ chế lưu `millis()` cũ và kiểm tra delta time.
2. **Quản lý RAM ESP32-C3**: RAM của ESP32-C3 rất hạn chế (~400KB). Mảng `_animeBuffer` và `_clockCanvas` (GFXcanvas16) đã chiếm dụng một phần. Nếu khởi tạo thêm các buffer hoặc String lớn, hãy cẩn thận dọn dẹp (Free / Delete) để tránh crash Heap.
3. **Thêm UI mới**: Nếu tạo thêm một mặt đồng hồ mới, hãy định nghĩa thêm trong `LayoutMode` (`types.h`), thêm logic vẽ trong hàm `drawStaticLayout` (phần nền không đổi) và `updateDynamicUI` (phần cập nhật mỗi giây/giây lẻ). 
4. **Hình ảnh tĩnh mới**: Ưu tiên sử dụng hàm vẽ đường thẳng/hình tròn hình học (như `IconManager`) thay vì nhét thêm mảng XBM/Bitmap nếu không cần thiết, vì bộ nhớ Flash cần để dành cho mảng `anime_frames.h`.
5. **Cập nhật Frame GIF**: Đừng sinh code Python bắt người dùng chạy máy tính. Hãy bảo người dùng đưa file ảnh vào file `converter.html` có sẵn ở thư mục gốc (nó đã tích hợp sẵn thuật toán RLE).
