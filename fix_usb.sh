#!/bin/bash

# 1. Nạp driver nếu chưa có
sudo insmod base_convert_driver.ko 2>/dev/null

# 2. Tìm ADDR bằng cách quét tất cả các thiết bị USB có ID abcd:1234
# Lệnh này tìm thư mục chứa file 'idVendor' có nội dung 'abcd'
DEV_PATH=$(grep -l "abcd" /sys/bus/usb/devices/*/idVendor 2>/dev/null | head -n 1 | cut -d/ -f1-6)

if [ -z "$DEV_PATH" ]; then
    echo " LỖI: lsusb thấy nhưng hệ thống sysfs chưa sẵn sàng. Hãy rút USB ra cắm lại!"
    exit 1
fi

# Lấy ID địa chỉ (ví dụ: 1-1) và thêm interface :1.0
ADDR=$(basename $DEV_PATH):1.0

echo " Đã tìm thấy USB tại địa chỉ Kernel: $ADDR"

# 3. Thực hiện Re-bind
# Ngắt khỏi driver cũ (nếu đang bám)
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind 2>/dev/null
# Gắn vào driver của bạn
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/bind 2>/dev/null

# 4. Cấp quyền file thiết bị
sudo chmod 666 /dev/base_convert 2>/dev/null

echo "Xong! Bây giờ bạn có thể chạy: sudo ./app"