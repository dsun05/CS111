ifneq ($(KERNELRELEASE),)
obj-m := proc_count.o
else
KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: default modules_install install clean

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif