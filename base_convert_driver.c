#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/usb.h>
#include <linux/string.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

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
    int mode;
    struct mutex lock;
};

static unsigned char aes_key[32] = "MySecretHardwareKey123456789012";

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

        //  BẮT LỖI TẠI ĐÂY: Chữ số phải nhỏ hơn hệ cơ số
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
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct scatterlist sg_in, sg_out;
    unsigned int dec;

    mutex_lock(&dev_data.lock);

    if(len > BUF_SIZE-1) len = BUF_SIZE-1;

    memset(dev_data.input, 0, BUF_SIZE);
    memset(dev_data.output, 0, BUF_SIZE);
    copy_from_user(dev_data.input, buf, len);

    if (dev_data.mode == MODE_CONVERT || dev_data.mode == 0) {
        dec = to_decimal(dev_data.input, dev_data.from);
        from_decimal(dec, dev_data.to, dev_data.output);
    } else {
        char *crypt_in;
        char *crypt_out;
        int ret;

        tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
        if (IS_ERR(tfm)) {
            printk(KERN_ERR "BC_USB: Allocate AES failed\n");
            mutex_unlock(&dev_data.lock);
            return -EFAULT;
        }

        req = skcipher_request_alloc(tfm, GFP_KERNEL);
        if (!req) {
            crypto_free_skcipher(tfm);
            mutex_unlock(&dev_data.lock);
            return -ENOMEM;
        }

        crypt_in = kmalloc(BUF_SIZE, GFP_KERNEL);
        crypt_out = kzalloc(BUF_SIZE, GFP_KERNEL);

        if (!crypt_in || !crypt_out) {
            if (crypt_in) kfree(crypt_in);
            if (crypt_out) kfree(crypt_out);
            skcipher_request_free(req);
            crypto_free_skcipher(tfm);
            mutex_unlock(&dev_data.lock);
            return -ENOMEM;
        }

        memcpy(crypt_in, dev_data.input, BUF_SIZE);

        if (crypto_skcipher_setkey(tfm, aes_key, 32)) {
            printk(KERN_ERR "BC_USB: Setkey failed\n");
            kfree(crypt_in);
            kfree(crypt_out);
            skcipher_request_free(req);
            crypto_free_skcipher(tfm);
            mutex_unlock(&dev_data.lock);
            return -EFAULT;
        }

        sg_init_one(&sg_in, crypt_in, BUF_SIZE);
        sg_init_one(&sg_out, crypt_out, BUF_SIZE);
        skcipher_request_set_crypt(req, &sg_in, &sg_out, BUF_SIZE, NULL);

        if (dev_data.mode == MODE_ENCRYPT) {
            ret = crypto_skcipher_encrypt(req);
        } else {
            ret = crypto_skcipher_decrypt(req);
        }

        if (ret < 0) {
            printk(KERN_ERR "BC_USB: Crypto op failed with error %d\n", ret);
        } else {
            memcpy(dev_data.output, crypt_out, BUF_SIZE);
        }

        kfree(crypt_in);
        kfree(crypt_out);
        skcipher_request_free(req);
        crypto_free_skcipher(tfm);
    }

    mutex_unlock(&dev_data.lock);
    return len;
}

ssize_t bc_read(struct file *f,char __user *buf,size_t len,loff_t *off){
    int l = (dev_data.mode == MODE_CONVERT || dev_data.mode == 0) ? strlen(dev_data.output) : BUF_SIZE;
    copy_to_user(buf, dev_data.output, l);
    return l;
}

long bc_ioctl(struct file *f, unsigned int cmd, unsigned long arg){
    int val;
    // Kiểm tra việc copy dữ liệu từ User Space
    if (copy_from_user(&val, (int*)arg, sizeof(int))) {
        return -EFAULT;
    }

    switch(cmd){
        case IOCTL_SET_FROM_BASE:
            if (val < 2 || val > 36) return -EINVAL;
            dev_data.from = val; 
            break;
        case IOCTL_SET_TO_BASE:
            if (val < 2 || val > 36) return -EINVAL;
            dev_data.to = val; 
            break;
        case IOCTL_SET_MODE:
            dev_data.mode = val;
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