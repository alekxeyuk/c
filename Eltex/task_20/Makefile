obj-m += first.o
KERNEL_V := $(shell uname -r)

PWD := $(CURDIR)

all:
	$(MAKE) -C /lib/modules/$(KERNEL_V)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KERNEL_V)/build M=$(PWD) clean