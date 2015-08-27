TARGET_BASE=test_libCmdline
ifeq ($(OS), Windows_NT)
  TARGET_EXTENSION=.exe
else
  TARGET_EXTENSION=.out
endif
LIB := $(TARGET_BASE)$(TARGET_EXTENSION)

SRCS = $(UNIT_TEST_BASE)/unity.c Test.c Test_Runner.c
SRCS += ../../libCmdline/ns_cmdline.c ../../libList/ns_list.c

override CFLAGS += -DTEST -std=gnu99 -pedantic -D'MEM_ALLOC=malloc' -D'MEM_FREE=free'

include ../../unittest_rules.mk


