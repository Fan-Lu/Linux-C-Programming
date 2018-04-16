obj-m := driver_testcase4.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	gcc -o TESTCASE4 TESTCASE4.c -lpthread
	

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~
	rm -rf TESTCASE4
	
