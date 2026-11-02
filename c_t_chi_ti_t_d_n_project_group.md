# ĐẶC TẢ CHI TIẾT BÀI TẬP LỚN (PROJECT GROUP SPECIFICATION)
**Môn học:** Vi Xử Lý – Vi Điều Khiển (CO3009) – HK1 (2026–2027)  
**Đề tài:** Hệ thống đèn giao thông 2 tuyến đường trên mạch XNUCLEO-F103RB  
**Giảng viên hướng dẫn:** TS. Nguyễn Hòa Hưng  
**Hạn nộp Báo cáo & Mã nguồn:** 04/12/2026 | **Demo:** Tuần 15/16  

---

## 1. Mục tiêu và Phạm vi Dự án
Xây dựng hệ thống điều khiển đèn giao thông cho ngã tư 2 tuyến đường hoạt động thời gian thực trên vi điều khiển **STM32F103RB** (kit **XNUCLEO-F103RB**). Hệ thống phải đáp ứng các yêu cầu kỹ thuật bắt buộc sau:
1. **Kiến trúc hướng tác vụ (Cooperative Scheduler $O(1)$):** Toàn bộ hệ thống vận hành trên bộ lập lịch với hàm `SCH_Update()` đạt độ phức tạp $O(1)$ (sử dụng độ trễ tương đối - Relative Delay), hỗ trợ đầy đủ cả tác vụ định kỳ (Periodic Task) và tác vụ chạy một lần (One-shot Task, `Period = 0`). Tuyệt đối không sử dụng `HAL_Delay()` trong vòng lặp chính.
2. **Kiến trúc phần mềm phân lớp (Layered Architecture):** Tách biệt hoàn toàn lớp Phần cứng (HAL), lớp Driver phụ trợ (`button`, `software_timer`, `traffic_light`, `dht20`, `lcd`) và lớp Ứng dụng (`fsm_automatic`, `fsm_manual`, `fsm_tuning`). Trong các file `fsm_*.c` không được phép chứa bất kỳ lời gọi hàm `HAL_...` nào.
3. **Đảm bảo an toàn giao thông tuyệt đối:** Không bao giờ xảy ra tình trạng cả 2 tuyến đường cùng sáng đèn Xanh trong mọi chế độ và mọi thời điểm chuyển giao.

---

## 2. Danh mục Thiết bị Ngoại vi
* **Mạch điều khiển chính:** Kit XNUCLEO-F103RB kèm mạch mở rộng Grove Shield.
* **Module đèn giao thông (x2):** Tuyến 1 (kết nối chân `D3 - D4`), Tuyến 2 (kết nối chân `D5 - D6`). Mỗi module nhận 2 tín hiệu điều khiển logic để giải mã ra 4 trạng thái: Đỏ (`RED`), Vàng (`AMBER`), Xanh (`GREEN`), và Tắt (`OFF`).
* **Module nút nhấn rời (x2 module = 4 nút):** Kết nối tại các chân `D7` (Nút 1), `D8` (Nút 2), `D9` (Nút 3), `D10` (Nút 4).
* **Nút nhấn và LED tích hợp trên mạch:** Nút `USER` màu xanh (`PC13`), LED xanh lá (`PA5`) và LED phụ (`PC9`).
* **Cảm biến môi trường DHT20 (x1):** Giao tiếp qua chuẩn I2C1 (địa chỉ 7-bit cố định: `0x38`), đo nhiệt độ ($^\circ\text{C}$) và độ ẩm tương đối ($\%\text{RH}$).
* **Màn hình LCD 16x2 I2C (x1):** Giao tiếp qua chuẩn I2C1, hiển thị chế độ hoạt động, bộ đếm ngược thời gian đèn và thông số nhiệt độ/độ ẩm.

---

## 3. Đặc tả Chức năng của 3 Chế độ Hoạt động

### 3.1. Trạng thái Khởi động (System Boot-up)
* Ngay sau khi cấp nguồn hoặc nhấn nút Reset:
  * Hệ thống khởi tạo GPIO, I2C1, TIM2 (ngắt chu kỳ $10\text{ ms}$) và bộ lập lịch Scheduler.
  * Kích hoạt một **One-shot Task** (`Period = 0`, `Delay = 150 ticks = 1.5s`) hiển thị thông điệp khởi động lên LCD:
    * Dòng 1: `CO3009 PROJECT  `
    * Dòng 2: `TRAFFIC SYSTEM  `
  * Sau $1.5\text{ giây}$, One-shot Task tự động chuyển hệ thống vào **Chế độ Tự động (`AUTOMATIC`)** tại pha khởi đầu `AUTO_RED1_GREEN2` và tự xóa khỏi hàng đợi Scheduler.
* **Thông số thời gian mặc định khi khởi động:**
  * Thời gian đèn Xanh: $T_{\text{Xanh}} = 5\text{ giây}$.
  * Thời gian đèn Vàng: $T_{\text{Vàng}} = 2\text{ giây}$.
  * Thời gian đèn Đỏ (tự động tính): $T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}} = 7\text{ giây}$.

### 3.2. Chế độ 1: Tự động (`AUTOMATIC MODE`)
* Hai tuyến đường hoạt động luân phiên tự động theo chu kỳ khép kín 4 pha:
  1. **Pha 1 (`AUTO_RED1_GREEN2`):** Tuyến 1 sáng đèn **Đỏ** (đếm ngược từ $T_{\text{Đỏ}}$), Tuyến 2 sáng đèn **Xanh** (đếm ngược từ $T_{\text{Xanh}}$). Kéo dài trong $T_{\text{Xanh}}\text{ giây}$.
  2. **Pha 2 (`AUTO_RED1_AMBER2`):** Tuyến 1 tiếp tục sáng đèn **Đỏ** (đếm ngược phần thời gian còn lại bằng $T_{\text{Vàng}}$), Tuyến 2 chuyển sang đèn **Vàng** (đếm ngược từ $T_{\text{Vàng}}$). Kéo dài trong $T_{\text{Vàng}}\text{ giây}$.
  3. **Pha 3 (`AUTO_GREEN1_RED2`):** Tuyến 1 chuyển sang đèn **Xanh** (đếm ngược từ $T_{\text{Xanh}}$), Tuyến 2 sáng đèn **Đỏ** (đếm ngược từ $T_{\text{Đỏ}}$). Kéo dài trong $T_{\text{Xanh}}\text{ giây}$.
  4. **Pha 4 (`AUTO_AMBER1_RED2`):** Tuyến 1 chuyển sang đèn **Vàng** (đếm ngược từ $T_{\text{Vàng}}$), Tuyến 2 tiếp tục sáng đèn **Đỏ**. Kéo dài trong $T_{\text{Vàng}}\text{ giây}$, sau đó lặp lại Pha 1.
* Cảm biến DHT20 được đọc định kỳ mỗi $2\text{ giây}$ và cập nhật nhiệt độ, độ ẩm lên dòng 2 của màn hình LCD.

### 3.3. Chế độ 2: Bằng tay có Pha Vàng chuyển tiếp 3 giây (`MANUAL MODE`)
* Cho phép người điều khiển chủ động phân luồng giao thông bằng nút nhấn nhưng vẫn **cưỡng bức chèn pha đèn Vàng $3\text{ giây}$** trước khi đảo chiều đèn Xanh/Đỏ để đảm bảo an toàn:
  * Khi đang ở trạng thái **Đỏ 1 – Xanh 2 (`MAN_RED1_GREEN2`)**: Hệ thống giữ nguyên trạng thái này vô thời hạn cho đến khi người dùng nhấn **Nút 2 (`D8`)**.
  * Ngay khi nhấn **Nút 2**: Hệ thống chuyển sang trạng thái trung gian **Đỏ 1 – Vàng 2 (`MAN_RED1_AMBER2`)** trong đúng **$3\text{ giây}$** (LCD đếm ngược `3s -> 2s -> 1s`). Trong $3\text{ giây}$ này, mọi thao tác nhấn Nút 2 đều bị bỏ qua. Hết $3\text{ giây}$, hệ thống tự động chuyển sang trạng thái giữ **Xanh 1 – Đỏ 2 (`MAN_GREEN1_RED2`)**.
  * Khi đang ở trạng thái **Xanh 1 – Đỏ 2 (`MAN_GREEN1_RED2`)**: Nếu nhấn **Nút 2 (`D8`)**, hệ thống chuyển sang trạng thái trung gian **Vàng 1 – Đỏ 2 (`MAN_AMBER1_RED2`)** trong **$3\text{ giây}$**, sau đó tự động chuyển về **Đỏ 1 – Xanh 2 (`MAN_RED1_GREEN2`)**.

### 3.4. Chế độ 3: Chỉnh thời gian (`TUNING MODE`) — Chỉ chỉnh $T_{\text{Xanh}}, T_{\text{Vàng}}$; Tự tính $T_{\text{Đỏ}}$
* **Nguyên lý ràng buộc hợp lệ:** Trong giao thông 2 tuyến đối xứng, để đèn 2 hướng luôn khớp pha mà không xảy ra xung đột, thời gian đèn Đỏ bắt buộc phải bằng tổng thời gian đèn Xanh và đèn Vàng của tuyến kia:
  $$T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}}$$
* **Quy trình chỉnh gồm 2 bước tuần tự:**
  1. **Bước 1 – Chỉnh thời gian đèn Xanh (`TUNING_GREEN`):**
     * Đèn Xanh của cả 2 tuyến chớp tắt với tần số $2\text{ Hz}$ (chu kỳ $500\text{ ms}$) để báo hiệu đang chỉnh thông số đèn Xanh.
     * Người dùng nhấn **Nút 2 (`D8`)** để tăng `temp_green` ($+1\text{s}$), nhấn **Nút 3 (`D9`)** để giảm `temp_green` ($-1\text{s}$). Hỗ trợ nhấn giữ (long press $> 1\text{s}$) để tăng/giảm nhanh mỗi $200\text{ ms}$. Giới hạn hợp lệ: $1\text{s} \le T_{\text{Xanh}} \le 90\text{s}$.
     * Màn hình LCD hiển thị đồng thời giá trị `temp_green` đang chỉnh và giá trị `temp_red` dự kiến ($\text{temp\_green} + \text{temp\_amber}$).
     * Nhấn **Nút 4 (`D10`)** để xác nhận $T_{\text{Xanh}}$ và chuyển sang Bước 2.
  2. **Bước 2 – Chỉnh thời gian đèn Vàng (`TUNING_AMBER`):**
     * Đèn Vàng của cả 2 tuyến chớp tắt chu kỳ $500\text{ ms}$.
     * Nhấn **Nút 2 (`D8`)** để tăng `temp_amber` ($+1\text{s}$), nhấn **Nút 3 (`D9`)** để giảm `temp_amber` ($-1\text{s}$). Giới hạn hợp lệ: $1\text{s} \le T_{\text{Vàng}} \le 9\text{s}$.
     * Nhấn **Nút 4 (`D10`)** để **Lưu & Áp dụng**:
       * Hệ thống cập nhật biến toàn cục: `time_green = temp_green`, `time_amber = temp_amber`, và `time_red = time_green + time_amber`.
       * Gọi **One-shot Task** hiển thị thông báo `"SAVED CONFIG!"` trên LCD trong $1.5\text{ giây}$.
       * Tự động khởi động lại chu kỳ **Chế độ Tự động (`AUTOMATIC`)** với bộ thông số thời gian mới.
  * **Cơ chế Hủy (Cancel):** Ở bất kỳ bước nào trong chế độ `TUNING`, nếu người dùng nhấn **Nút 1 (`D7`)**, mọi thay đổi trên biến tạm `temp_green`, `temp_amber` sẽ bị hủy bỏ, hệ thống giữ nguyên thời gian cũ và quay về `AUTOMATIC`.

---

## 4. Bảng Ma trận Chức năng các Nút nhấn

| Nút nhấn | Chân Phần cứng | Chế độ Tự động (`AUTOMATIC`) | Chế độ Bằng tay (`MANUAL`) | Chế độ Chỉnh thời gian (`TUNING`) |
| :--- | :---: | :--- | :--- | :--- |
| **Nút 1** | `D7` (`PA8`) | Chuyển sang chế độ `MANUAL` | Chuyển sang chế độ `TUNING` | **Hủy bỏ (Cancel)** thay đổi và quay về `AUTOMATIC` |
| **Nút 2** | `D8` (`PA9`) | Không sử dụng | **Đổi làn đường** (Kích hoạt pha Vàng $3\text{s}$ rồi đảo làn Xanh/Đỏ) | **Tăng ($+1\text{s}$)** giá trị thời gian đang chọn (Nhấn giữ: tăng nhanh) |
| **Nút 3** | `D9` (`PC7`) | Không sử dụng | Không sử dụng | **Giảm ($-1\text{s}$)** giá trị thời gian đang chọn (Nhấn giữ: giảm nhanh) |
| **Nút 4** | `D10` (`PB6`) | Không sử dụng | Không sử dụng | **Xác nhận / Lưu:** Chuyển từ chỉnh Xanh $\rightarrow$ chỉnh Vàng $\rightarrow$ Lưu & về `AUTOMATIC` |
| **Nút USER** | `PC13` (On-board) | Đổi thông tin dòng 2 LCD (Môi trường DHT20 $\leftrightarrow$ Thông số cài đặt) | **Reset khẩn cấp:** Lập tức đưa hệ thống về đầu chế độ `AUTOMATIC` | **Khôi phục mặc định:** Đặt lại $T_{\text{Xanh}}=5\text{s}, T_{\text{Vàng}}=2\text{s}, T_{\text{Đỏ}}=7\text{s}$ |

---

## 5. Thiết kế Giao diện Màn hình LCD 16x2

Màn hình LCD có kích thước 2 hàng $\times$ 16 cột ký tự. Định dạng hiển thị được chuẩn hóa như sau:

| Chế độ / Trạng thái | Nội dung Dòng 1 (16 ký tự) | Nội dung Dòng 2 (16 ký tự) | Giải thích hiển thị |
| :--- | :--- | :--- | :--- |
| **Khởi động (One-shot 1.5s)** | `CO3009 PROJECT  ` | `TRAFFIC SYSTEM  ` | Hiển thị 1 lần duy nhất khi mới bật nguồn |
| **Tự động (`AUTOMATIC`)** | `AUTO  R1:07 G2:05` | `T:28.5C  H:65.2%` | Dòng 1: Trạng thái & đếm ngược 2 tuyến. Dòng 2: Nhiệt độ & Độ ẩm DHT20 |
| **Bằng tay – Giữ làn (`MANUAL`)** | `MANUAL MODE     ` | `L1:RED   L2:GRN ` | Hiển thị màu đèn hiện tại của Tuyến 1 (`L1`) và Tuyến 2 (`L2`) |
| **Bằng tay – Đang chuyển làn (3s)** | `SWITCHING LANE..` | `YELLOW WAIT: 03s` | Báo hiệu đang trong pha đèn Vàng chuyển tiếp $3\text{s}$ |
| **Chỉnh Đèn Xanh (`TUNING_GREEN`)** | `TUNE GREEN:  05s` | `AUTO RED  :  07s` | Dòng 1: $T_{\text{Xanh}}$ đang chỉnh. Dòng 2: $T_{\text{Đỏ}}$ tự động cộng theo |
| **Chỉnh Đèn Vàng (`TUNING_AMBER`)** | `TUNE AMBER:  02s` | `AUTO RED  :  07s` | Dòng 1: $T_{\text{Vàng}}$ đang chỉnh. Dòng 2: $T_{\text{Đỏ}}$ tự động cộng theo |
| **Thông báo Lưu (One-shot 1.5s)** | `  SAVED CONFIG! ` | `R:07s G:05s Y:02` | Xác nhận bộ thông số mới trước khi về `AUTOMATIC` |