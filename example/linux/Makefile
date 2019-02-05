override CFLAGS += -g -Wall -Wno-unused-function

all:
	gcc main.c $(CFLAGS) -I ../../mbed-client-cli -lncurses -I ../../source/ns_list_internal  ../../source/ns_cmdline.c -I. ../../source/ns_list_internal/ns_list.c -o cli
