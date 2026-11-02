# KẾ HOẠCH TRIỂN KHAI & ĐẶC TẢ KỸ THUẬT HỆ THỐNG
**Dự án:** Hệ thống đèn giao thông 2 tuyến đường trên mạch XNUCLEO-F103RB  
**Môn học:** Vi Xử Lý – Vi Điều Khiển (CO3009)  

---

## PHẦN 1: ĐẶC TẢ PHẦN CỨNG & CẤU HÌNH `.ioc` (TRA CỨU TỪ SCHEMATIC)

### 1.1. Bảng ánh xạ chân Arduino / Grove sang STM32F103RB
Dữ liệu được trích xuất từ tài liệu `MB1136 Nucleo-64 Schematic` (Sheet 2 `MCU_LQFP64` và Sheet 4 `Extension connectors CN5, CN9`):

| Thiết bị ngoại vi | Vị trí kết nối | Chân STM32F103RB | Cấu hình GPIO / Ngoại vi | User Label (`.ioc`) | Ghi chú kỹ thuật từ Schematic |
| :--- | :---: | :---: | :--- | :--- | :--- |
| **Đèn giao thông 1 - Bit A** | Chân **D3** | **PB3** | `GPIO_Output` (Push-Pull) | `TL1_A` | Chân 55 LQFP64. Giải phóng khỏi `JTDO` khi chọn `Serial Wire` |
| **Đèn giao thông 1 - Bit B** | Chân **D4** | **PB5** | `GPIO_Output` (Push-Pull) | `TL1_B` | Chân 57 LQFP64 (CN9 - Pin 5) |
| **Đèn giao thông 2 - Bit A** | Chân **D5** | **PB4** | `GPIO_Output` (Push-Pull) | `TL2_A` | Chân 56 LQFP64. Giải phóng khỏi `JNTRST` khi chọn `Serial Wire` |
| **Đèn giao thông 2 - Bit B** | Chân **D6** | **PB10** | `GPIO_Output` (Push-Pull) | `TL2_B` | Chân 29 LQFP64 (CN9 - Pin 7) |
| **Nút nhấn 1 (Mode)** | Chân **D7** | **PA8** | `GPIO_Input` (`Pull-up`) | `BTN_1` | Chân 41 LQFP64 (CN9 - Pin 8) |
| **Nút nhấn 2 (Switch/Up)** | Chân **D8** | **PA9** | `GPIO_Input` (`Pull-up`) | `BTN_2` | Chân 42 LQFP64 (CN5 - Pin 1) |
| **Nút nhấn 3 (Down)** | Chân **D9** | **PC7** | `GPIO_Input` (`Pull-up`) | `BTN_3` | Chân 38 LQFP64 (CN5 - Pin 2) |
| **Nút nhấn 4 (Confirm)** | Chân **D10** | **PB6** | `GPIO_Input` (`Pull-up`) | `BTN_4` | Chân 58 LQFP64 (CN5 - Pin 3) |
| **Nút nhấn USER (B1)** | On-board | **PC13** | `GPIO_Input` (`No pull`) | `BTN_USER` | Đã có sẵn trở kéo lên `R30 = 4.7kΩ` trên mạch. Nhấn = Mức `0` |
| **LED tích hợp trên mạch** | On-board | **PA5**, **PC9** | `GPIO_Output` (Push-Pull) | `LED_PA5`, `LED_PC9` | Dùng kiểm thử phần cứng và báo nhịp Scheduler |
| **Bus I2C1 (DHT20 & LCD)** | Khe I2C (Đỏ) / **D15, D14** | **PB8 (SCL)**, **PB9 (SDA)** | `I2C1_SCL`, `I2C1_SDA` | *(Mặc định I2C1)* | CN5 Pin 10 (`PB8`) & Pin 9 (`PB9`). Cần remap `I2C1` sang `PB8/PB9` |

### 1.2. Cấu hình Clock, Timer và Debug trong STM32CubeIDE
1. **Board Selector:** Chọn đúng **`NUCLEO-F103RB`** (Chip `STM32F103RBT6`).
2. **System Core $\rightarrow$ SYS $\rightarrow$ Debug:** Bắt buộc chọn **`Serial Wire`** (giữ chân `PA13/SWDIO` và `PA14/SWCLK` để nạp code nhiều lần).
3. **Timers $\rightarrow$ TIM2:**
   * Nguồn xung nhịp nội (Internal Clock): Giả sử $f_{\text{TIM2}} = 8\text{ MHz}$ (hoặc $72\text{ MHz}$ tùy cấu hình Clock Tree).
   * Công thức tính chu kỳ ngắt $T_{\text{tick}} = 10\text{ ms} = 0.01\text{ s}$:
     $$T_{\text{tick}} = \frac{(\text{PSC} + 1) \times (\text{ARR} + 1)}{f_{\text{TIM2}}}$$
   * Với $f_{\text{TIM2}} = 8\text{ MHz}$: Chọn $\text{Prescaler (PSC)} = 7999$, $\text{Counter Period (ARR)} = 9 \implies T_{\text{tick}} = \frac{8000 \times 10}{8 \times 10^6} = 0.01\text{ s} = 10\text{ ms}$.
   * Với $f_{\text{TIM2}} = 64\text{ MHz}$: Chọn $\text{PSC} = 6399$, $\text{ARR} = 99 \implies T_{\text{tick}} = 10\text{ ms}$.
   * Bật tích **TIM2 global interrupt** trong thẻ **NVIC Settings**.

---

## PHẦN 2: KIẾN TRÚC PHẦN MỀM & THIẾT KẾ CÁC MODULE

### 2.1. Sơ đồ phân lớp kiến trúc (Layered Architecture)
```text
+-------------------------------------------------------------------+
|                   LỚP ỨNG DỤNG (APPLICATION / FSM)                |
|        fsm_automatic.c  |   fsm_manual.c   |   fsm_tuning.c       |
|         (Chỉ gọi API của Lớp Driver - Không chứa hàm HAL)         |
+-------------------------------------------------------------------+
|               LỚP MODULE PHỤ TRỢ & LẬP LỊCH (MIDDLEWARE)          |
|  scheduler.c (O(1)) | software_timer.c | button.c | traffic_light |
|                     |     dht20.c      |  lcd.c   |   global.c    |
+-------------------------------------------------------------------+
|                LỚP TRỪU TƯỢNG PHẦN CỨNG (STM32 HAL)               |
|         HAL_GPIO        |       HAL_I2C       |      HAL_TIM      |
+-------------------------------------------------------------------+
|                   PHẦN CỨNG MẠCH XNUCLEO-F103RB                   |
+-------------------------------------------------------------------+
```

### 2.2. Thiết kế Scheduler $O(1)$ (`scheduler.h / .c`)
* **Cấu trúc dữ liệu lựa chọn:** Mảng tĩnh được sắp xếp theo thứ tự thời gian chạy tăng dần (Sorted Array with Relative Delay), kích thước `SCH_MAX_TASKS = 20`.
* **Nguyên lý đạt $O(1)$ tại `SCH_Update()`:**
  * Mỗi phần tử `SCH_tasks_G[i]` lưu biến `Delay` là **độ trễ tương đối** so với tác vụ đứng ngay trước nó (`SCH_tasks_G[i-1]`).
  * Trong ngắt Timer $10\text{ ms}$, `SCH_Update()` chỉ cần giảm `SCH_tasks_G[0].Delay` của phần tử đầu tiên. Khi `SCH_tasks_G[0].Delay == 0`, bật cờ `RunMe = 1`. Độ phức tạp thời gian là **$O(1)$ tuyệt đối**.
* **Độ phức tạp các hàm:**
  * `SCH_Init()`: $O(1)$ – Khởi tạo biến đếm `SCH_count = 0`.
  * `SCH_Add_Task(pFunc, DELAY, PERIOD)`: $O(n)$ – Duyệt mảng tìm vị trí chèn sao cho tổng `Delay` các phần tử đứng trước nhỏ hơn hoặc bằng `DELAY`, trừ `DELAY` cho tổng đó, dịch phải các phần tử phía sau và trừ `Delay` của phần tử kế tiếp cho `Delay` vừa chèn.
  * `SCH_Delete_Task(taskID)`: $O(n)$ – Tìm vị trí task cần xóa, cộng dồn `Delay` của task bị xóa vào task đứng ngay sau nó (để bảo toàn thời điểm chạy của các task phía sau), rồi dịch trái mảng.
  * `SCH_Dispatch_Tasks()`: Thực thi trong `while(1)`. Khi `SCH_tasks_G[0].RunMe > 0`, thực thi `(*pTask)()`, lưu lại `Period` và `pTask`, xóa phần tử đầu (`SCH_Delete_Task`). Nếu `Period > 0` (task tuần hoàn), gọi lại `SCH_Add_Task(pTask, Period, Period)`. Nếu `Period == 0` (**One-shot task**), không thêm lại.

### 2.3. Thiết kế các Module Phụ trợ (Drivers)
1. **Module `software_timer.h / .c`:**
   * Cung cấp 4 bộ đếm mềm độc lập (`MAX_TIMERS = 4`):
     * `Timer 0`: Đếm thời gian chuyển pha đèn trong `AUTOMATIC` và đếm pha Vàng $3\text{s}$ trong `MANUAL`.
     * `Timer 1`: Nhịp đếm ngược $1\text{ giây}$ ($1000\text{ ms}$) để giảm biến hiển thị giây trên LCD.
     * `Timer 2`: Nhịp chớp tắt đèn $500\text{ ms}$ ($2\text{ Hz}$) trong chế độ `TUNING`.
   * Giao diện hàm: `void setTimer(uint8_t index, uint32_t duration_ms)`, `void clearTimer(uint8_t index)`, `uint8_t isTimerExpired(uint8_t index)`, `void timer_run(void)`.
2. **Module `button.h / .c`:**
   * Quản lý 5 nút nhấn (`NUM_BUTTONS = 5`: `BTN_1`, `BTN_2`, `BTN_3`, `BTN_4`, `BTN_USER`).
   * **Cơ chế chống dội (Debounce):** Sử dụng 3 thanh ghi dịch mẫu (`KeyReg0`, `KeyReg1`, `KeyReg2`) đọc mỗi $10\text{ ms}$. Chỉ khi 3 lần đọc liên tiếp giống nhau (`KeyReg0 == KeyReg1 && KeyReg1 == KeyReg2`) mới xác nhận trạng thái ổn định.
   * **Cơ chế Nhấn giữ (Long Press / Auto-repeat):** Nếu nút được giữ liên tục sau $100\text{ ticks}$ ($1\text{ giây}$), bật cờ nhấn lặp lại mỗi $20\text{ ticks}$ ($200\text{ ms}$) để tăng/giảm nhanh thời gian trong chế độ `TUNING`.
   * Giao diện hàm: `void button_reading(void)`, `uint8_t isButtonPressed(uint8_t index)`, `void clearAllButtons(void)`.
3. **Module `traffic_light.h / .c`:**
   * Định nghĩa enum màu đèn: `LIGHT_OFF = 0`, `LIGHT_RED`, `LIGHT_AMBER`, `LIGHT_GREEN`.
   * Giao diện hàm: `void traffic_set_lane1(TrafficColor_t color)`, `void traffic_set_lane2(TrafficColor_t color)`, `void traffic_toggle_both(TrafficColor_t color)`.
4. **Module `dht20.h / .c`:**
   * Trình tự đọc không gây treo (Non-blocking State Machine trong task $2\text{s}$ hoặc tách 2 pha): Gửi lệnh đo `0xAC, 0x33, 0x00` đến địa chỉ `0x38 << 1`, sau ít nhất $80\text{ ms}$ đọc về $6\text{ bytes}$ dữ liệu.
   * Công thức chuyển đổi 20-bit raw data ($S_{\text{RH}}, S_T$) theo Datasheet DHT20:
     $$\text{Humidity (\%RH)} = \frac{S_{\text{RH}}}{2^{20}} \times 100\%$$
     $$\text{Temperature (}^\circ\text{C)} = \frac{S_T}{2^{20}} \times 200 - 50$$
5. **Module `lcd.h / .c`:**
   * Điều khiển LCD 16x2 qua module mở rộng I2C PCF8574 (chế độ 4-bit nibbles) hoặc Grove RGB LCD.
   * Cung cấp các hàm hiển thị đóng gói sẵn cho FSM: `lcd_show_auto(...)`, `lcd_show_manual(...)`, `lcd_show_tuning(...)`, `lcd_show_popup(...)`.

---

## PHẦN 3: PHÂN TÍCH CHUYỂN ĐỔI GIỮA CÁC CHẾ ĐỘ LỚN (TRỌNG TÂM BÁO CÁO MỤC 2.3)

### 3.1. Chuyển từ `AUTOMATIC` sang `MANUAL` (Sự kiện: Nhấn `BTN_1`)
1. **Trạng thái đèn của 2 tuyến ngay sau khi chuyển:**
   * Nếu đang ở `AUTO_RED1_GREEN2` $\rightarrow$ Chuyển sang `MAN_RED1_GREEN2` (Tuyến 1 Đỏ, Tuyến 2 Xanh).
   * Nếu đang ở `AUTO_GREEN1_RED2` $\rightarrow$ Chuyển sang `MAN_GREEN1_RED2` (Tuyến 1 Xanh, Tuyến 2 Đỏ).
   * **Nếu chuyển đúng lúc đèn đang Vàng (`AUTO_RED1_AMBER2` hoặc `AUTO_AMBER1_RED2`):** Hệ thống chuyển tương ứng vào trạng thái chuyển tiếp `MAN_RED1_AMBER2` hoặc `MAN_AMBER1_RED2` và nạp `setTimer(0, 3000)` để hoàn thành nốt $3\text{ giây}$ đèn Vàng trước khi dừng ở trạng thái Đỏ - Xanh tiếp theo.
2. **Xử lý Software Timer & Scheduler:** Gọi `clearTimer(0)` và `clearTimer(1)` (trừ trường hợp kế thừa pha Vàng), gọi `clearAllButtons()` để xóa các cờ nút nhấn tồn đọng.
3. **Đảm bảo an toàn:** Không bao giờ gán trực tiếp 2 tuyến cùng Xanh; nếu đang ở pha Vàng thì bắt buộc đi hết pha Vàng mới mở làn Xanh mới.

### 3.2. Chuyển từ `MANUAL` sang `TUNING` (Sự kiện: Nhấn `BTN_1`)
1. **Trạng thái đèn của 2 tuyến:** Tắt trạng thái phân luồng tĩnh, chuyển cả 2 tuyến sang nhấp nháy đèn Xanh chu kỳ $500\text{ ms}$ (báo hiệu bước chỉnh thời gian Xanh `TUNING_GREEN`).
2. **Xử lý biến đếm và Timer:** Sao chép `temp_green = time_green` và `temp_amber = time_amber`. Khởi tạo `setTimer(2, 500)` để điều khiển nhịp chớp đèn.
3. **Hiển thị LCD:** Chuyển sang màn hình `TUNE GREEN` ở dòng 1 và `AUTO RED` ở dòng 2.

### 3.3. Chuyển từ `TUNING` về `AUTOMATIC` (Sự kiện: Nhấn `BTN_4` ở bước cuối hoặc `BTN_1` để Hủy)
1. **Kiểm tra tính hợp lệ của thời gian mới:**
   * Do người dùng chỉ chỉnh $T_{\text{Xanh}} \in [1, 90]\text{s}$ và $T_{\text{Vàng}} \in [1, 9]\text{s}$, còn hệ thống tự tính $T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}}$ nên ràng buộc $T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}}$ luôn thỏa mãn 100%.
2. **Áp dụng thời gian mới vào chu kỳ tự động:**
   * Nếu kết thúc bằng `BTN_4` (Lưu): Cập nhật `time_green`, `time_amber`, `time_red`, gọi One-shot Task hiển thị popup `"SAVED CONFIG!"` trong $1.5\text{s}$.
   * Nếu kết thúc bằng `BTN_1` (Hủy): Giữ nguyên `time_green`, `time_amber`, `time_red` cũ.
   * Khởi tạo lại trạng thái `AUTO_RED1_GREEN2`, đặt `count_lane1 = time_red`, `count_lane2 = time_green`, gọi `setTimer(0, time_green * 1000)` và `setTimer(1, 1000)`.

---

## PHẦN 4: LỘ TRÌNH THỰC HIỆN 5 TUẦN (TIMELINE)

| Tuần | Mốc thời gian | Nội dung công việc chính | Sản phẩm đầu ra (Deliverables) |
| :-: | :---: | :--- | :--- |
| **Tuần 1** | `01/11 – 06/11` | Lập repo GitHub, tạo project `.ioc` (`NUCLEO-F103RB`, `Serial Wire`), kiểm thử phần cứng (LED, bảng chân lý Đèn, Nút, I2C Scan), chốt toàn bộ file `.h` | Bảng chân lý 2-bit của đèn, địa chỉ I2C của LCD & DHT20, nhánh `dev` có sẵn khung `.h` |
| **Tuần 2** | `07/11 – 13/11` | Thành viên A code `scheduler.c` $O(1)$. Thành viên B code `traffic_light.c` & `lcd.c`. Thành viên C vẽ sơ đồ FSM và code `global.c`, `fsm_automatic.c` | Scheduler chạy mượt 2 LED + 1 One-shot task; LCD in được chữ; sơ đồ FSM hoàn chỉnh |
| **Tuần 3** | `14/11 – 20/11` | Thành viên A code `software_timer.c`, `button.c`. Thành viên B code `dht20.c`. Thành viên C code `fsm_manual.c` và `fsm_tuning.c` | Đọc nút chống dội tốt, DHT20 hiển thị nhiệt độ/độ ẩm lên LCD mỗi $2\text{s}$ |
| **Tuần 4** | `21/11 – 27/11` | Merge các nhánh `feature/*` vào `dev`. Tích hợp toàn hệ thống, kiểm thử các cung chuyển chế độ và tình huống biên | Hệ thống chạy hoàn chỉnh 3 chế độ trên mạch thật, không còn lệnh HAL trong `fsm_*.c` |
| **Tuần 5** | `28/11 – 04/12` | Stress test chạy liên tục $>30\text{ phút}$, hoàn thiện Báo cáo PDF theo mẫu, gắn tag `v1.0-final` trên nhánh `main` và nộp bài LMS | File `CO3009_..._Report.pdf`, Tag `v1.0-final`, video demo dự phòng |