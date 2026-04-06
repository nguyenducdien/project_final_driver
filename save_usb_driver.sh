#!/bin/bash

# Script: tháo ráp nóng driver USB để lưu file và trả lại trạng thái cho bc_usb

# Chờ đọc nội dung từ pipe (stdin) lưu vào biến
CONTENT=$(cat)

# 1. Tìm ADDR của USB
DEV_PATH=$(grep -l "abcd" /sys/bus/usb/devices/*/idVendor 2>/dev/null | head -n 1 | cut -d/ -f1-6)
if [ -z "$DEV_PATH" ]; then
    echo "LỖI: Không tìm thấy thiết bị USB bc_usb!"
    exit 1
fi
ADDR=$(basename $DEV_PATH):1.0

echo "Đang gỡ Kernel module bc_usb khỏi $ADDR ..."
# 2. Ngắt khỏi bc_usb
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/unbind >/dev/null

echo "Đang gắn module lưu trữ usb-storage..."
# 3. Gắn vào usb-storage
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/bind >/dev/null

# 4. Chờ hệ thống nhận đĩa (block device)
sleep 2

# sysfs: /sys/bus/usb/devices/$ADDR/host*/target*/*/block/sd*
BLOCK_DEV=$(ls /sys/bus/usb/devices/$ADDR/host*/target*/*/block/ 2>/dev/null | head -n 1 | xargs basename 2>/dev/null)

if [ -z "$BLOCK_DEV" ]; then
    echo "LỖI: Trình điều khiển đã thay đổi nhưng không tìm thấy ổ đĩa vật lý (Block Device)!"
    # Rebind lại cho an toàn
    echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind >/dev/null
    echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/bind >/dev/null
    exit 1
fi

PARTITION="/dev/${BLOCK_DEV}1"
# Hoặc nếu không có phân vùng 1 thì ghi thẳng vào /dev/sdX
if [ ! -b "$PARTITION" ]; then
    PARTITION="/dev/${BLOCK_DEV}"
fi

# 5. Gắn ổ đĩa
MOUNT_DIR="/tmp/bc_usb_mount"
sudo mkdir -p "$MOUNT_DIR"
echo "Đang Mount $PARTITION vào $MOUNT_DIR..."
sudo mount "$PARTITION" "$MOUNT_DIR" 2>/dev/null

if [ $? -ne 0 ]; then
    echo "LỖI: Mount file system thất bại!"
else
    # 6. Ghi nội dung vào USB
    echo "$CONTENT" | sudo tee -a "$MOUNT_DIR/ketqua_chuyendoi.txt" >/dev/null
    sync
    sudo umount "$MOUNT_DIR"
    echo "Ghi file THÀNH CÔNG!"
fi

# 7. Trả lại Driver ban đầu
echo "Đang tháo ổ lưu trữ và gắn lại Kernel module biến đổi bc_usb..."
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/usb-storage/unbind >/dev/null
echo "$ADDR" | sudo tee /sys/bus/usb/drivers/bc_usb/bind >/dev/null

sleep 1
sudo chmod 666 /dev/base_convert 2>/dev/null
echo "Hoàn tất! Chương trình đã sẵn sàng."