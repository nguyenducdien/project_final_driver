#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 
#include "base_convert_ioctl.h"

#define DEV "/dev/base_convert"
#define LOG_FILE "ketqua_chuyendoi.txt"

// === HÀM KIỂM TRA TÍNH HỢP LỆ CỦA SỐ NHẬP VÀO ===
int is_valid(char *str, int base) {
    for (int i = 0; i < strlen(str); i++) {
        int digit;
        char c = str[i];
        
        // Chuyển ký tự sang giá trị số tương ứng
        if (isdigit(c)) 
            digit = c - '0';
        else if (isalpha(c)) 
            digit = toupper(c) - 'A' + 10;
        else 
            return 0; // Ký tự lạ (%, #, @...) là sai ngay

        // Nếu chữ số >= hệ cơ số (ví dụ hệ 2 mà có số 2) -> SAI
        if (digit >= base) return 0;
    }
    return 1; // Hợp lệ
}

int main() {
    int fd;
    int from, to;
    char input[100], output[100] = {0};

    // 1. Kết nối với Driver
    fd = open(DEV, O_RDWR);
    if (fd < 0) {
        printf("Lỗi: Không thể mở thiết bị!\n");
        printf("Gợi ý: Hãy chạy lệnh: ./fix_usb.sh\n");
        return 1;
    }

    printf("=== HỆ THỐNG CHUYỂN ĐỔI CƠ SỐ QUA USB DRIVER ===\n");

    // 2. Nhập hệ cơ số
    printf(">> Nhập hệ cơ số NGUỒN (2-36): "); scanf("%d", &from);
    printf(">> Nhập hệ cơ số ĐÍCH (2-36): "); scanf("%d", &to);

    // 3. Kiểm tra hệ cơ số qua Driver
    if (ioctl(fd, IOCTL_SET_FROM_BASE, &from) < 0 || ioctl(fd, IOCTL_SET_TO_BASE, &to) < 0) {
        printf("Lỗi: Hệ cơ số không hợp lệ (2-36)!\n");
        close(fd);
        return 1;
    }

    // 4. Nhập số và BẮT LỖI LOGIC
    printf(">> Nhập số cần chuyển: "); scanf("%s", input);

    if (!is_valid(input, from)) {
        printf("\n LỖI: Số '%s' chứa chữ số không hợp lệ trong hệ cơ số %d!\n", input, from);
        printf("Gợi ý: Hệ 2 chỉ gồm 0-1, hệ 10 gồm 0-9, hệ 16 gồm 0-9 và A-F.\n");
        close(fd);
        return 1;
    }

    // 5. Gửi dữ liệu xuống Driver sau khi đã kiểm tra an toàn
    write(fd, input, strlen(input));
    read(fd, output, 100);

    printf("\nKẾT QUẢ TỪ DRIVER: %s\n", output);

    // 6. Ghi kết quả ra file log
    FILE *f = fopen(LOG_FILE, "a");
    if (f) {
        fprintf(f, "------------------------------------------\n");
        fprintf(f, "Số nhập vào: %s (Hệ %d)\n", input, from);
        fprintf(f, "Kết quả trả về: %s (Hệ %d)\n", output, to);
        fprintf(f, "Trạng thái: Thành công qua USB Driver\n");
        fclose(f);
        printf("Đã lưu kết quả vào file: %s\n", LOG_FILE);
    }

    close(fd);
    printf("==========================================\n");

    return 0;
}