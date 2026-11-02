# BẢNG PHÂN CÔNG NHIỆM VỤ & CHECKLIST CHO 3 THÀNH VIÊN (A, B, C)
**Dự án:** Hệ thống đèn giao thông 2 tuyến đường trên mạch XNUCLEO-F103RB  
**Nguyên tắc cốt lõi:** Mỗi thành viên làm việc trên nhánh Git riêng, sở hữu độc quyền các file `.c/.h` được giao và tự commit bằng tài khoản GitHub cá nhân để đảm bảo điểm đánh giá đóng góp (10% tổng điểm).

---

## 1. Tổng quan Phân bổ Trách nhiệm (33.3% / Thành viên)

| Hạng mục | Thành viên A (Kernel & Input) | Thành viên B (Hardware & I2C Drivers) | Thành viên C (Application & FSM) |
| :--- | :--- | :--- | :--- |
| **Nhánh Git phụ trách** | `feature/scheduler-input` | `feature/hardware-drivers` | `feature/fsm-logic` |
| **Các file `.h / .c` sở hữu** | `scheduler.h/.c`<br>`software_timer.h/.c`<br>`button.h/.c`<br>`main.c` *(tích hợp Scheduler)* | `<project>.ioc`<br>`traffic_light.h/.c`<br>`dht20.h/.c`<br>`lcd.h/.c` | `global.h/.c`<br>`fsm_automatic.h/.c`<br>`fsm_manual.h/.c`<br>`fsm_tuning.h/.c` |
| **Phần Báo cáo Word phụ trách** | - Mục 1.3 (Sơ đồ khối hệ thống)<br>- Mục 3.2 (Hiện thực Scheduler $O(1)$)<br>- Mục 3.3.1 & 3.3.2 (Timer & Button)<br>- Mục 5 (Hướng dẫn build & sử dụng) | - Mục 1.1 (Tổng quan & Ảnh chụp mạch)<br>- Mục 3.1 (Kết nối phần cứng & Bảng tra chân)<br>- Mục 3.3.3 & 3.3.4 (Đèn, DHT20, LCD) | - Mục 1.2 (Các chức năng hệ thống)<br>- Mục 2.1, 2.2, 2.3 (Thiết kế & Phân tích chuyển chế độ)<br>- Mục 3.4 & 3.5 (Thiết kế & Hiện thực 3 FSM)<br>- Mục 4 (Tổng kết) |
| **Tỷ lệ đóng góp** | **33.3%** | **33.3%** | **33.3%** |

---

## 2. Nhiệm vụ Chi tiết của Thành viên A (System Kernel & Input Engineer)

### 2.1. Phần Lập trình (Code)
1. **Module `scheduler.h / .c`:**
   * Cài đặt cấu trúc `sTask` gồm `pTask`, `Delay`, `Period`, `RunMe`, `TaskID`.
   * Hiện thực danh sách task bằng mảng sắp xếp theo độ trễ tương đối (Relative Delay) để hàm `SCH_Update()` chỉ xử lý phần tử đầu tiên `SCH_tasks_G[0]` với độ phức tạp **$O(1)$**.
   * Hiện thực đầy đủ `SCH_Init()`, `SCH_Add_Task()`, `SCH_Delete_Task()`, `SCH_Dispatch_Tasks()`.
   * Hỗ trợ **One-shot Task** (`Period == 0`): Sau khi chạy xong trong `SCH_Dispatch_Tasks()`, tự động xóa khỏi mảng mà không thêm lại.
2. **Module `software_timer.h / .c`:**
   * Xây dựng mảng `timer_counter[MAX_TIMERS]` và `timer_flag[MAX_TIMERS]`.
   * Lưu ý quy đổi đơn vị: Khi truyền `duration` tính bằng `ms` vào `setTimer(index, duration_ms)`, chia cho `TICK_MS` ($10\text{ ms}$) để ra số nhịp đếm.
3. **Module `button.h / .c`:**
   * Đọc trạng thái 5 chân GPIO (`PA8`, `PA9`, `PC7`, `PB6`, `PC13`) mỗi $10\text{ ms}$.
   * Cài đặt bộ lọc chống dội 3 mẫu liên tiếp (`KeyReg0`, `KeyReg1`, `KeyReg2`).
   * Phát hiện sự kiện nhấn đơn (`isButtonPressed(i)` trả về `1` đúng 1 lần rồi tự xóa cờ về `0`) và nhấn giữ $> 1\text{ giây}$ (tự động bật lại cờ mỗi $200\text{ ms}$).
4. **Tích hợp trong `main.c`:**
   * Gọi `HAL_TIM_Base_Start_IT(&htim2)` và đặt `SCH_Update()` trong hàm ngắt `HAL_TIM_PeriodElapsedCallback()`.
   * Đăng ký các task vào Scheduler trước vòng lặp `while(1)` và gọi `SCH_Dispatch_Tasks()` trong `while(1)`.

### 2.2. Phần Viết Báo cáo & Kiểm thử
* Vẽ sơ đồ khối kiến trúc 4 lớp của hệ thống (Mục 1.3).
* Viết phân tích so sánh Array vs Linked List, giải thích thuật toán chèn Relative Delay trong `SCH_Add_Task` và chứng minh `SCH_Update` đạt $O(1)$ (Mục 3.2).
* Điền bảng danh sách các task trong hệ thống và công thức tính `PSC`, `ARR` của `TIM2` (Mục 3.2.3).

---

## 3. Nhiệm vụ Chi tiết của Thành viên B (Hardware & Peripheral Driver Engineer)

### 3.1. Phần Phần cứng & Lập trình (Code)
1. **Khởi tạo `.ioc` & Kiểm thử phần cứng (Bước 2 & Bước 3):**
   * Tạo dự án từ **Board Selector (`NUCLEO-F103RB`)**, bật **`SYS -> Debug: Serial Wire`**.
   * Cấu hình GPIO (`PB3, PB5, PB4, PB10, PA8, PA9, PC7, PB6, PA5, PC9, PC13`), remap `I2C1` sang `PB8 (SCL) / PB9 (SDA)`, cấu hình `TIM2` ngắt $10\text{ ms}$.
   * Chạy code kiểm thử phần cứng ban đầu để xác định:
     * Bảng chân lý 4 tổ hợp `(0,0), (0,1), (1,0), (1,1)` của 2 module đèn giao thông.
     * Mức logic khi nhấn của 4 nút rời và nút USER.
     * Chạy I2C Scanner tìm địa chỉ 7-bit của LCD và DHT20.
2. **Module `traffic_light.h / .c`:**
   * Viết hàm `traffic_set(uint8_t lane, TrafficColor_t color)` xuất đúng tổ hợp 2 chân GPIO cho Tuyến 1 (`TL1_A`, `TL1_B`) và Tuyến 2 (`TL2_A`, `TL2_B`).
3. **Module `dht20.h / .c`:**
   * Viết hàm khởi tạo và đọc cảm biến DHT20 qua `hi2c1` (địa chỉ `0x38 << 1`), tính toán ra 2 biến số thực hoặc số nguyên nhân 10 (`temperature`, `humidity`).
4. **Module `lcd.h / .c`:**
   * Viết driver giao tiếp LCD qua I2C (`lcd_init`, `lcd_clear`, `lcd_goto_XY`, `lcd_send_string`, `lcd_send_num`).
   * Xây dựng hàm cập nhật màn hình `lcd_update_task()` để hiển thị nội dung theo biến trạng thái toàn cục mà không làm nhấp nháy màn hình (chỉ ghi đè các ký tự thay đổi, hạn chế gọi lệnh xóa toàn màn hình liên tục).

### 3.2. Phần Viết Báo cáo & Kiểm thử
* Chụp ảnh hệ thống phần cứng thực tế sau khi lắp ráp hoàn chỉnh (Hình 1.1).
* Điền bảng tra cứu chân Arduino $\rightarrow$ STM32, bảng kết quả tổ hợp 2 tín hiệu đèn và trích đoạn hàm `MX_GPIO_Init()` (Mục 3.1).
* Trình bày giao thức đọc I2C của DHT20, công thức chuyển đổi nhiệt độ/độ ẩm và bảng tổng hợp API của các module phụ trợ (Mục 3.3.3 & 3.3.4).

---

## 4. Nhiệm vụ Chi tiết của Thành viên C (Application & FSM Engineer)

### 4.1. Phần Thiết kế & Lập trình (Code)
1. **Module `global.h / .c`:**
   * Định nghĩa toàn bộ các mã trạng thái của hệ thống (`INIT`, `AUTO_RED1_GREEN2`, `AUTO_RED1_AMBER2`, `AUTO_GREEN1_RED2`, `AUTO_AMBER1_RED2`, `MAN_RED1_GREEN2`, `MAN_RED1_AMBER2`, `MAN_GREEN1_RED2`, `MAN_AMBER1_RED2`, `TUNING_GREEN`, `TUNING_AMBER`).
   * Khai báo các biến dùng chung: `status`, `time_red`, `time_amber`, `time_green`, `temp_amber`, `temp_green`, `count_lane1`, `count_lane2`.
2. **Module `fsm_automatic.h / .c`:**
   * Hiện thực máy trạng thái 4 pha tự động, cập nhật biến đếm ngược `count_lane1`, `count_lane2` mỗi $1\text{ giây}$.
   * Bắt sự kiện nhấn `BTN_1` để chuyển sang `MANUAL` (xử lý an toàn cả khi đang ở pha đèn Vàng).
3. **Module `fsm_manual.h / .c`:**
   * Hiện thực máy trạng thái điều khiển bằng tay có chèn pha đèn Vàng chuyển tiếp $3\text{ giây}$ (`MAN_RED1_AMBER2` và `MAN_AMBER1_RED2`) khi nhấn `BTN_2`.
   * Bắt sự kiện nhấn `BTN_1` chuyển sang `TUNING`, hoặc `BTN_USER` để Reset về `AUTOMATIC`.
4. **Module `fsm_tuning.h / .c`:**
   * Hiện thực 2 bước chỉnh `TUNING_GREEN` và `TUNING_AMBER` với hiệu ứng chớp đèn $2\text{ Hz}$.
   * Xử lý tăng/giảm bằng `BTN_2` / `BTN_3`, hủy bằng `BTN_1`, và xác nhận lưu bằng `BTN_4` (tự động tính $T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}}$, kích hoạt One-shot Task báo `"SAVED CONFIG!"` trên LCD trong $1.5\text{s}$ rồi về `AUTOMATIC`).
   * **Kiểm tra nghiêm ngặt:** Đảm bảo trong 3 file `fsm_*.c` không có bất kỳ từ khóa `HAL_` nào.

### 4.2. Phần Viết Báo cáo & Kiểm thử
* Viết bảng mô tả các chức năng và bảng ma trận nút nhấn (Mục 1.2 & 2.1).
* Dùng phần mềm Draw.io vẽ 4 sơ đồ máy trạng thái: Sơ đồ chuyển Mode tổng quát (Hình 2.1), FSM Tự động (Hình 3.1), FSM Bằng tay (Hình 3.2), FSM Chỉnh thời gian (Hình 3.3) kèm các bảng chuyển trạng thái.
* Viết chi tiết **Mục 2.3 (Phân tích chuyển đổi giữa các chế độ lớn)** — phần chiếm **30% tổng điểm** của bài tập lớn.

---

## 5. Checklist Đồng bộ Nhóm qua từng Giai đoạn

- [ ] **Chốt Giao diện Header (Cuối Tuần 1):** Cả 3 thành viên cùng duyệt và commit các file `.h` lên nhánh `dev`. Sau mốc này, không tự ý đổi tên hàm trong file `.h` nếu chưa thông báo cho cả nhóm.
- [ ] **Kiểm tra Lịch sử Commit (Hàng tuần):** Mỗi thành viên phải có tối thiểu 5–10 commits có ý nghĩa rõ ràng trên GitHub từ tài khoản cá nhân của mình.
- [ ] **Kiểm tra Tách lớp Phần cứng (Cuối Tuần 4):** Tìm kiếm từ khóa `HAL_` trong toàn bộ các file `fsm_*.c`. Kết quả bắt buộc phải bằng `0`.