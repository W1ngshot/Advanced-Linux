obj-m += int_stack.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc kernel_stack.c -o kernel_stack

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f kernel_stack
