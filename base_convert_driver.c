#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/usb.h>
#include <linux/string.h>

#include "base_convert_ioctl.h"

#define DEVICE_NAME "base_convert"
#define CLASS_NAME  "bc_class"
#define BUF_SIZE 128

#define USB_VENDOR_ID  0xabcd
#define USB_PRODUCT_ID 0x1234
struct base_convert_dev {
    char input[BUF_SIZE];
    char output[BUF_SIZE];
    int from;
    int to;
    struct mutex lock;
};

static struct base_convert_dev dev_data;
static dev_t dev_num;
static struct cdev bc_cdev;
static struct class *bc_class;
static struct device *bc_device;

/* ================= CONVERT ================= */

int char_to_val(char c){
    if(c>='0'&&c<='9') return c-'0';
    if(c>='A'&&c<='F') return c-'A'+10;
    return -1;
}

// Giả sử hàm nhận vào chuỗi 'str' và hệ cơ số 'base'
long to_decimal(char *str, int base) {
    long res = 0;
    int i;
    for (i = 0; i < strlen(str); i++) {
        int digit;
        char c = str[i];

        // Chuyển ký tự thành giá trị số
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
        else return -1; // Ký tự không hợp lệ

        // 🔥 BẮT LỖI TẠI ĐÂY: Chữ số phải nhỏ hơn hệ cơ số
        if (digit >= base) {
            printk(KERN_ERR "BC_USB: Chu so %d khong hop le cho he co so %d!\n", digit, base);
            return -EINVAL; // Trả về mã lỗi Invalid Argument (-22)
        }

        res = res * base + digit;
    }
    return res;
}
void from_decimal(unsigned int val,int base,char *out){
    char tmp[BUF_SIZE];
    int i=0,j=0;

    if(val==0){ strcpy(out,"0"); return; }

    while(val){
        int r=val%base;
        tmp[i++]=(r<10)?('0'+r):('A'+r-10);
        val/=base;
    }
    while(i) out[j++]=tmp[--i];
    out[j]=0;
}

/* ================= FILE OPS ================= */

ssize_t bc_write(struct file *f,const char __user *buf,size_t len,loff_t *off){
    mutex_lock(&dev_data.lock);

    if(len > BUF_SIZE-1) len = BUF_SIZE-1;

    copy_from_user(dev_data.input,buf,len);
    dev_data.input[len]=0;

    unsigned int dec=to_decimal(dev_data.input,dev_data.from);
    from_decimal(dec,dev_data.to,dev_data.output);

    mutex_unlock(&dev_data.lock);
    return len;
}

ssize_t bc_read(struct file *f,char __user *buf,size_t len,loff_t *off){
    int l = strlen(dev_data.output);
    copy_to_user(buf,dev_data.output,l);
    return l;
}

long bc_ioctl(struct file *f, unsigned int cmd, unsigned long arg){
    int val;
    // Kiểm tra việc copy dữ liệu từ User Space
    if (copy_from_user(&val, (int*)arg, sizeof(int))) {
        return -EFAULT;
    }

    // ĐOẠN QUAN TRỌNG: Bắt lỗi hệ cơ số tại đây
    if (val < 2 || val > 36) {
        printk(KERN_INFO "BC_USB: Hệ cơ số %d nằm ngoài phạm vi (2-36)\n", val);
        return -EINVAL; // Trình biên dịch sẽ gửi mã lỗi về cho lệnh ioctl ở App
    }

    switch(cmd){
        case IOCTL_SET_FROM_BASE:
            dev_data.from = val; 
            break;
        case IOCTL_SET_TO_BASE:
            dev_data.to = val; 
            break;
    }
    return 0;
}

static struct file_operations fops={
    .owner=THIS_MODULE,
    .read=bc_read,
    .write=bc_write,
    .unlocked_ioctl=bc_ioctl
};

/* ================= USB ================= */

static int bc_probe(struct usb_interface *intf,const struct usb_device_id *id){
    printk("USB CONNECTED\n");
    bc_device=device_create(bc_class,NULL,dev_num,NULL,DEVICE_NAME);
    return 0;
}

static void bc_disconnect(struct usb_interface *intf){
    printk("USB DISCONNECTED\n");
    device_destroy(bc_class,dev_num);
}

static struct usb_device_id table[]={
    {USB_DEVICE(USB_VENDOR_ID,USB_PRODUCT_ID)},
    {}
};

MODULE_DEVICE_TABLE(usb,table);

static struct usb_driver usb_drv={
    .name="bc_usb",
    .id_table=table,
    .probe=bc_probe,
    .disconnect=bc_disconnect
};

/* ================= INIT ================= */

static int __init init_mod(void){
    alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);

    cdev_init(&bc_cdev,&fops);
    cdev_add(&bc_cdev,dev_num,1);

    bc_class=class_create(CLASS_NAME);

    mutex_init(&dev_data.lock);
    dev_data.from=10;
    dev_data.to=2;

    usb_register(&usb_drv);

    printk("Driver loaded\n");
    return 0;
}

static void __exit exit_mod(void){
    usb_deregister(&usb_drv);

    class_destroy(bc_class);
    cdev_del(&bc_cdev);
    unregister_chrdev_region(dev_num,1);

    printk("Driver removed\n");
}

module_init(init_mod);
module_exit(exit_mod);

MODULE_LICENSE("GPL");