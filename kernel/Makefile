ifneq ($(KERNELRELEASE),)

obj-m += xt_TCPWIN.o
xt_TCPWIN-y := xt_TWIN.o

else

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

all:	modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=$(DESTDIR)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
