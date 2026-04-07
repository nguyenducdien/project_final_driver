#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "base_convert_ioctl.h"

#define BUF_SIZE 128

// Hàm phụ trợ chuyển đổi ký tự HEX sang Decimal
int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Tham so thieu!\nCach dung: %s <mode: 1=Encrypt, 2=Decrypt> <Text/HEX>\n", argv[0]);
        return 1;
    }

    int mode = atoi(argv[1]);
    char *input_str = argv[2];
    
    int fd = open("/dev/base_convert", O_RDWR);
    if (fd < 0) {
        printf("Ban chua cam USB hoặc Driver chua load!\n");
        return 1;
    }

    // Truyền cờ Mã Hóa / Giải mã cho Kernel Driver
    if (ioctl(fd, IOCTL_SET_MODE, &mode) < 0) {
        printf("Loi truyền IOCTL_SET_MODE\n");
        close(fd);
        return 1;
    }

    char buf[BUF_SIZE] = {0};

    if (mode == 1) { 
        // CHẾ ĐỘ MÃ HÓA (ENCRYPT)
        strncpy(buf, input_str, BUF_SIZE - 1);
        write(fd, buf, BUF_SIZE); // Gửi plain-text
        read(fd, buf, BUF_SIZE);  // Nhận Binary CipherText
        
        // Trả ra dạng Chuỗi HEX (Vì dữ liệu mã hóa nhị phân sẽ làm hỏng terminal)
        for(int i = 0; i < BUF_SIZE; i++) {
            printf("%02X", (unsigned char)buf[i]);
        }
        printf("\n");
        
    } else if (mode == 2) { 
        // CHẾ ĐỘ GIẢI MÃ (DECRYPT)
        int len = strlen(input_str);
        // Biến ngược chuỗi HEX ảo thành Dữ liệu Nhị phân (Binary)
        for(int i = 0; i < BUF_SIZE && i*2 < len; i++) {
            buf[i] = (hex_char_to_int(input_str[i*2]) << 4) | hex_char_to_int(input_str[i*2+1]);
        }
        write(fd, buf, BUF_SIZE); // Gửi Binary CipherText
        read(fd, buf, BUF_SIZE);  // Nhận text thường gốc
        
        printf("%s\n", buf); // In lại văn bản
    }

    // Reset trở về chế độ Đổi cơ số mặc định, để các luồng khác không bị ảnh hưởng
    int convert_mode = 0;
    ioctl(fd, IOCTL_SET_MODE, &convert_mode);

    close(fd);
    return 0;
}
