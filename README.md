<<<<<<< HEAD
# Hệ Thống Chuyển Đổi Cơ Số Bằng USB (USB Base Converter)

Dự án này là một trình điều khiển nhân hệ điều hành Linux (Kernel Driver) giúp giao tiếp với một thiết bị USB vật lý để thực hiện tính toán chuyển đổi cơ số (từ hệ 2-36). Nó đi kèm với một Giao diện người dùng (GUI) đẹp mắt hỗ trợ cả việc chuyển đổi trên chip USB lẫn chức năng lưu trữ kết quả thẳng vào trong thẻ nhớ USB (Hot-swapping).

---

## 🛠 YÊU CẦU MÔI TRƯỜNG (PREREQUISITES)

Hệ thống được thiết kế dành cho Linux. Đảm bảo bạn đã cài đặt các gói cần thiết sau:

**1. Đối với dòng Debian/Ubuntu (Mint, Kali...):**
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) python3-pip python3-tk -y
sudo pip3 install customtkinter --break-system-packages
```

**2. Đối với dòng RedHat/CentOS:**
```bash
sudo yum install epel-release -y
sudo yum install kernel-devel kernel-headers gcc make python3-pip python3-tkinter -y
sudo pip3 install customtkinter
```

---

## 🚀 HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY ỨNG DỤNG

### Bước 1: Biên dịch mã nguồn C
Tiến hành buid ra file Kernel Module (`.ko`) và app giao tiếp CLI bằng lệnh:
```bash
make
```
*(Nếu thành công sẽ sinh ra 2 file quan trọng: `base_convert_driver.ko` và `cli_app`)*

### Bước 2: Chuẩn bị USB Phần cứng
- Cắm thiết bị USB vào máy tính (Đảm bảo mã Vendor ID của USB trong mã nguồn C đang khớp với phần cứng thật. Mặc định đang thiết lập để quét là `abcd`).

### Bước 3: Nạp Driver & Ép quyền điều khiển USB
Sử dụng script sửa lỗi tự động cực mạnh. Nó sẽ nạp Driver vào nền hệ điều hành OS, sau đó giật lấy quyền quản lý USB từ tay Linux để cấp cho nhân tính toán:
```bash
sudo bash fix_usb.sh
```

### Bước 4: Khởi động Giao diện Cửa sổ
Bật giao diện lên ứng dụng:
```bash
sudo python3 gui_app.py
```

---

## 🎯 HƯỚNG DẪN SỬ DỤNG GIAO DIỆN

Khi Cửa sổ xuất hiện, bạn có 3 tính năng mạnh mẽ:
1. **[CHUYỂN ĐỔI]:** Nhập đầy đủ 3 ô Hệ Nguồn, Hệ Đích, Con số. Nhấn để Kernel xử lý phép toán dưới nền và trả kết quả về bảng Nhật Ký đen.
2. **[LƯU VÀO USB]:** Bạn click vào đây, hệ thống sẽ ngắt Module Tính toán ➜ hô biến USB thành Ổ đĩa lưu trữ ➜ Copy tất cả những gì đang hiện trên màn hình đen ném vào trong file `ketqua_chuyendoi.txt` của USB ➜ Tự động lắp ráp USB lại thành chế độ Kernel Tính toán như ban đầu. Đỉnh cao công nghệ tàng hình!
3. **[ĐỌC TỪ USB]:** Truy cập siêu tốc vào file Text đang nằm tận sâu trong ruột chiếc USB và bưng trọn bộ lịch sử in thẳng lên màn hình đen cho bạn đọc.

---

## 🧹 DỌN DẸP HỆ THỐNG
Sau khi làm việc xong, muốn gỡ bỏ hoàn toàn file đã build:
```bash
sudo rmmod base_convert_driver
make clean
```
=======
Các bước chạy
 ./fix_usb.sh 
==>  sudo ./app
>>>>>>> 365d87ed11532699c1c837a936a4870c9af5b5ab
