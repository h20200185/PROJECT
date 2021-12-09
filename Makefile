obj-m += mymodule.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc  -o mcpUsr userMcp.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
