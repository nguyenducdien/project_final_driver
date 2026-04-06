#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 
#include "base_convert_ioctl.h"

#define DEV "/dev/base_convert"

int is_valid(char *str, int base) {
    for (int i = 0; i < strlen(str); i++) {
        int digit;
        char c = str[i];
        if (isdigit(c)) digit = c - '0';
        else if (isalpha(c)) digit = toupper(c) - 'A' + 10;
        else return 0;
        if (digit >= base) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("ERROR: Invalid arguments.\n");
        return 1;
    }

    int from = atoi(argv[1]);
    int to = atoi(argv[2]);
    char *input = argv[3];
    char output[100] = {0};

    if (!is_valid(input, from)) {
        printf("Vui long nhap dung so cho he so %d.\n", from);
        return 1;
    }

    int fd = open(DEV, O_RDWR);
    if (fd < 0) {
        printf("Ban chua cam USB! Vui long cam USB\n");
        return 1;
    }

    if (ioctl(fd, IOCTL_SET_FROM_BASE, &from) < 0 || ioctl(fd, IOCTL_SET_TO_BASE, &to) < 0) {
        printf("ERROR: Invalid base.\n");
        close(fd);
        return 1;
    }

    write(fd, input, strlen(input));
    read(fd, output, 100);

    // ONLY print the result so GUI can capture it cleanly
    printf("%s\n", output);

    close(fd);
    return 0;
}