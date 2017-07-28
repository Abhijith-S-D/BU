obj-m := kprobe_example.o
obj-m += jprobe_example.o
obj-m += kretprobe_example.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -f *.mod.c *.ko *.o
