#!/bin/bash

# Script: tháo ráp nóng driver USB để ĐỌC file
STATUS=0

# 1. Tìm ADDR của USB
DEV_PATH=$(grep -l "abcd" /sys/bus/usb/devices/*/idVendor 2>/dev/null | head -n 1 | cut -d/ -f1-6)
if [ -z "$DEV_PATH" ]; then
    echo "LỖI: Không tìm thấy thiết bị USB!" >&2
    exit 1
fi
ADDR=$(basename $DEV_PATH):1.0

# 2. Ngắt khỏi bc_usb
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/unbind >/dev/null

# 3. Gắn vào usb-storage
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/bind >/dev/null

# 4. Chờ hệ thống nhận đĩa
sleep 2
BLOCK_DEV=$(ls /sys/bus/usb/devices/$ADDR/host*/target*/*/block/ 2>/dev/null | head -n 1 | xargs basename 2>/dev/null)

if [ -z "$BLOCK_DEV" ]; then
    echo "LỖI: Không đọc được ổ đĩa vật lý (Block)! Cắm lại đi." >&2
    echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind >/dev/null
    echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/bind >/dev/null
    exit 1
fi

PARTITION="/dev/${BLOCK_DEV}1"
if [ ! -b "$PARTITION" ]; then
    PARTITION="/dev/${BLOCK_DEV}"
fi

# 5. Gắn ổ đĩa
MOUNT_DIR="/tmp/bc_usb_mount"
sudo mkdir -p "$MOUNT_DIR"
sudo mount "$PARTITION" "$MOUNT_DIR" 2>/dev/null

if [ $? -ne 0 ]; then
    echo "LỖI: Mount file system thất bại!" >&2
    STATUS=1
else
    # 6. Đọc nội dung
    if [ -f "$MOUNT_DIR/ketqua_chuyendoi.txt" ]; then
        # In đúng file ra stdout, mấy cái message cảnh báo lỗi được lọc ra stderr rồi
        cat "$MOUNT_DIR/ketqua_chuyendoi.txt"
    else
        echo "LỖI: File ketqua_chuyendoi.txt chưa tồn tại trên USB!" >&2
        STATUS=1
    fi
    sudo umount "$MOUNT_DIR"
fi

# 7. Trả lại Driver ban đầu cho phép tiếp tục chuyển đổi cơ số tiếp được
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind >/dev/null
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/bind >/dev/null

sleep 1
sudo chmod 666 /dev/base_convert 2>/dev/null

if [ "$STATUS" = "1" ]; then
    exit 1
fi
echo "Hoàn tất thao tác đọc!" >&2