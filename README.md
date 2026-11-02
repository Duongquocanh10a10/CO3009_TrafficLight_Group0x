# HỆ THỐNG ĐÈN GIAO THÔNG 2 TUYẾN ĐƯỜNG TRÊN MẠCH XNUCLEO-F103RB
**Môn học:** Vi Xử Lý – Vi Điều Khiển (CO3009) – Học kỳ 1 (2026–2027)  
**Giảng viên hướng dẫn:** TS. Nguyễn Hòa Hưng  
**Nền tảng phần cứng:** Kit XNUCLEO-F103RB (Vi điều khiển STM32F103RBT6)  

---

## 1. Thành viên Nhóm & Phân công Nhiệm vụ

| STT | Họ và tên | MSSV | Tài khoản GitHub | Nhánh làm việc chính | Module phụ trách độc quyền | Đóng góp |
| :-: | :--- | :---: | :---: | :---: | :--- | :-: |
| 1 | **[Họ và tên A]** | `[MSSV_A]` | `@username_A` | `feature/scheduler-input` | `scheduler`, `software_timer`, `button`, `main.c` | 33.3% |
| 2 | **[Họ và tên B]** | `[MSSV_B]` | `@username_B` | `feature/hardware-drivers` | File `.ioc`, `traffic_light`, `dht20`, `lcd` | 33.3% |
| 3 | **[Họ và tên C]** | `[MSSV_C]` | `@username_C` | `feature/fsm-logic` | `global`, `fsm_automatic`, `fsm_manual`, `fsm_tuning` | 33.3% |

---

## 2. Cấu trúc Thư mục Dự án

```text
<ten-repo>/
├── Core/
│   ├── Inc/
│   │   ├── main.h                  # Header cấu hình chân GPIO từ STM32CubeMX
│   │   ├── global.h                # Biến toàn cục, hằng số thời gian và mã trạng thái FSM
│   │   ├── scheduler.h             # Bộ lập lịch tác vụ Cooperative Scheduler O(1)
│   │   ├── software_timer.h        # Bộ đếm thời gian mềm cho các máy trạng thái
│   │   ├── button.h                # Đọc 5 nút nhấn, chống dội 3 mẫu và phát hiện nhấn giữ
│   │   ├── traffic_light.h         # Điều khiển 2 module đèn giao thông (D3-D4, D5-D6)
│   │   ├── dht20.h                 # Giao tiếp I2C1 đọc cảm biến nhiệt độ & độ ẩm DHT20
│   │   ├── lcd.h                   # Giao tiếp I2C1 điều khiển màn hình LCD 16x2
│   │   ├── fsm_automatic.h         # Máy trạng thái Chế độ Tự động (Automatic Mode)
│   │   ├── fsm_manual.h            # Máy trạng thái Chế độ Bằng tay (Manual Mode - Pha Vàng 3s)
│   │   └── fsm_tuning.h            # Máy trạng thái Chế độ Chỉnh thời gian (Tuning Mode)
│   └── Src/
│       ├── main.c                  # Khởi tạo phần cứng, ngắt TIM2 10ms và vòng lặp Dispatch
│       ├── global.c
│       ├── scheduler.c
│       ├── software_timer.c
│       ├── button.c
│       ├── traffic_light.c
│       ├── dht20.c
│       ├── lcd.c
│       ├── fsm_automatic.c
│       ├── fsm_manual.c
│       └── fsm_tuning.c
├── Drivers/                        # Thư viện chuẩn CMSIS và STM32F1xx_HAL_Driver
├── <ten-du-an>.ioc                 # File cấu hình phần cứng STM32CubeMX
├── .gitignore                      # Loại trừ thư mục biên dịch (Debug/, Release/, .metadata/...)
└── README.md                       # Tài liệu hướng dẫn build, vận hành và quy chuẩn Git
```

---

## 3. Quy trình Làm việc trên GitHub & Chống Xung đột (Merge Conflict)

### 3.1. Nguyên tắc Phân vùng File (Tuyệt đối tuân thủ)
1. **Mỗi người chỉ sửa file của mình:** Thành viên nào chỉ được phép chỉnh sửa các file `.c/.h` thuộc cột **Module phụ trách độc quyền** ở Mục 1.
2. **Bảo vệ file `.ioc` và `main.c`:**
   * Chỉ **Thành viên B** được phép thay đổi cấu hình trong file `.ioc` và bấm *Generate Code*.
   * Mọi đoạn code viết thêm trong `main.c` bắt buộc phải đặt bên trong cặp thẻ `/* USER CODE BEGIN ... */` và `/* USER CODE END ... */`.
3. **Không commit thay đổi nháp khi test:** Nếu bạn sửa tạm `main.c` để test module cá nhân, hãy chạy lệnh `git checkout -- Core/Src/main.c` trước khi `git commit`.

### 3.2. Các lệnh Git chuẩn hàng ngày cho Thành viên

* **Bước 1 – Clone dự án và tạo nhánh lần đầu:**
  ```bash
  git clone https://github.com/<tai-khoan>/<ten-repo>.git
  cd <ten-repo>
  git checkout dev
  git checkout -b feature/<ten-nhanh-cua-ban>
  ```

* **Bước 2 – Đồng bộ code mới nhất từ nhánh `dev` trước khi làm việc:**
  ```bash
  git checkout dev
  git pull origin dev
  git checkout feature/<ten-nhanh-cua-ban>
  git merge dev
  ```

* **Bước 3 – Commit và đẩy code lên nhánh cá nhân (sau khi Build `Ctrl + B` báo 0 lỗi):**
  ```bash
  git status
  git add Core/Inc/<ten_file>.h Core/Src/<ten_file>.c
  git commit -m "<ten_module>: <mo_ta_ngan_gon_thay_doi>"
  git push -u origin feature/<ten-nhanh-cua-ban>
  ```

* **Bước 4 – Gộp code vào nhánh `dev`:**
  * Mở GitHub $\rightarrow$ Tạo **Pull Request (PR)** từ nhánh `feature/...` vào nhánh `dev`.
  * Nhờ 1 thành viên khác trong nhóm kiểm tra và nhấn **Merge Pull Request**.

---

## 4. Hướng dẫn Build và Nạp Chương trình

> ⚠️ **LƯU Ý QUAN TRỌNG VỀ PHẦN CỨNG:**
> Mạch **XNUCLEO-F103RB** sử dụng chip **STM32F103RB**. Khi cấu hình trong file `.ioc`, bắt buộc phải chọn board **NUCLEO-F103RB** và thiết lập **System Core $\rightarrow$ SYS $\rightarrow$ Debug: Serial Wire**. Nếu để *No Debug*, mạch sẽ bị khóa cổng nạp SWD sau lần nạp đầu tiên.

1. Mở **STM32CubeIDE** $\rightarrow$ Chọn **File** $\rightarrow$ **Open Projects from File System...** $\rightarrow$ Chọn thư mục gốc của repository.
2. Nhấn **`Ctrl + B`** (*Project $\rightarrow$ Build Project*) để biên dịch toàn bộ mã nguồn.
3. Cắm cáp USB kết nối cổng ST-LINK của mạch XNUCLEO-F103RB với máy tính.
4. Nhấn **`F11`** (*Run $\rightarrow$ Debug*) hoặc nút **Run** màu xanh để nạp chương trình xuống mạch.

---

## 5. Hướng dẫn Vận hành Hệ thống

Sau khi nạp chương trình hoặc nhấn nút Reset màu đen (`B2`), màn hình LCD hiển thị thông báo khởi động trong $1.5\text{ giây}$ rồi tự động vào **Chế độ Tự động (`AUTOMATIC`)** với thông số mặc định: Đỏ = $7\text{s}$, Xanh = $5\text{s}$, Vàng = $2\text{s}$.

* **Nút 1 (`D7 - PA8`):** Chuyển đổi tuần tự giữa 3 chế độ: `AUTOMATIC` $\rightarrow$ `MANUAL` $\rightarrow$ `TUNING` $\rightarrow$ `AUTOMATIC` (Nếu nhấn khi đang ở `TUNING` sẽ hủy thay đổi).
* **Nút 2 (`D8 - PA9`):**
  * Ở chế độ `MANUAL`: Kích hoạt chuyển làn đường (Tự động bật đèn Vàng $3\text{ giây}$ trước khi đảo chiều Xanh/Đỏ).
  * Ở chế độ `TUNING`: Tăng giá trị thời gian đang chọn ($+1\text{s}$), nhấn giữ $>1\text{s}$ để tăng nhanh.
* **Nút 3 (`D9 - PC7`):**
  * Ở chế độ `TUNING`: Giảm giá trị thời gian đang chọn ($-1\text{s}$), nhấn giữ $>1\text{s}$ để giảm nhanh.
* **Nút 4 (`D10 - PB6`):**
  * Ở chế độ `TUNING`: Nhấn lần 1 để xác nhận $T_{\text{Xanh}}$ và chuyển sang chỉnh $T_{\text{Vàng}}$. Nhấn lần 2 để tự động tính $T_{\text{Đỏ}} = T_{\text{Xanh}} + T_{\text{Vàng}}$, lưu cấu hình và quay về chế độ `AUTOMATIC`.
* **Nút USER (`PC13` - Nút màu xanh trên mạch):**
  * Ở chế độ `AUTOMATIC`: Chuyển đổi nội dung hiển thị dòng 2 trên LCD.
  * Ở chế độ `MANUAL`: Reset khẩn cấp về đầu chu kỳ `AUTOMATIC`.
  * Ở chế độ `TUNING`: Khôi phục bộ thời gian mặc định ($7\text{s} - 5\text{s} - 2\text{s}$).

**Link Video Demo hoạt động thực tế:** `[Chèn link YouTube/Google Drive video demo của nhóm tại đây]`
