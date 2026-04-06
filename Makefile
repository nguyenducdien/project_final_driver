obj-m += base_convert_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc -o app user_app.c
	gcc -o cli_app cli_app.c

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f app cli_app