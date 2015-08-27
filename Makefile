SRCS := src/ns_cmdline.c
LIB := libCmdline.a

override CFLAGS += DHAVE_DEBUG

include ../library_rules.mk
