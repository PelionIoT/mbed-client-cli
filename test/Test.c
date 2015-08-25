/*
 * Copyright (c) 2014-2015 ARM. All rights reserved.
 */

/**
* \file \test_libTrace\Test.c
*
* \brief Unit tests for libTrace
*/
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "unity.h"
#include "ns_cmdline.h"

#define BUFSIZE 1024
char buf[BUFSIZE] = {0};
#define INIT_BUF()  memset(buf, 0, BUFSIZE)
int cmd_dummy(int argc, char *argv[])
{
    return 0;
}


void myprint(const char *fmt, va_list ap)
{
    vsnprintf(buf + strlen(buf), BUFSIZE - strlen(buf), fmt, ap);
    //printf("\nMYPRINT: %s\n", buf); //for test test
}
void input(const char *str)
{
    while (*str != 0) {
        cmd_char_input(*str++);
    }
}

#define REQUEST(x)          input(x);INIT_BUF();cmd_char_input('\r');
#define RESPONSE(x)         "\n"x"\n\r\x1B[2K/> \x1B[1D"
#define CMDLINE(x)          "\r\x1b[2K/>"x"\x1b[1D"

#define FORWARD             "C"
#define BACKWARD            "D"
#define CMDLINE_CUR(x, cursor, dir)  "\r\x1b[2K/>"x"\x1b["cursor""dir
#define CLEAN()             cmd_char_input('\r');INIT_BUF();

//vt100 keycodes
#define HOME()      input("\x1b[1~")
#define INSERT()    input("\x1b[2~")
#define DELETE()    input("\x1b[3~")
#define BACKSPACE() input("\x7f")
#define LEFT()      input("\x1b[D")
#define LEFT_N(n)   for(int i=0;i<n;i++) LEFT();
#define RIGHT()     input("\x1b[C")
#define RIGHT_N(n)  for(int i=0;i<n;i++) RIGHT()
#define UP()        input("\x1b[A")
#define DOWN()      input("\x1b[B")
#define ESC()       input("\x03")
#define PAGE_DOWN() input("\x1b[6~")
#define PAGE_UP()   input("\x1b[5~")

void cmd_ready_cb(int retcode)
{
    cmd_next(retcode);
}

/* Unity test code starts */
void setUp(void)
{
    cmd_init(&myprint);
    cmd_set_ready_cb(cmd_ready_cb);
    INIT_BUF();
}

void tearDown(void)
{
    INIT_BUF();
    cmd_free();
}

void test_param_0(void)
{
}
void test_param_1(void)
{
    char *argv[] = { "cmd", "p1", "p2", "p3", "p4", "p5" };
    int idx = cmd_parameter_index(6, argv, "p4");
    TEST_ASSERT_EQUAL_INT(4, idx);

    idx = cmd_parameter_index(6, argv, "p6");
    TEST_ASSERT_EQUAL_INT(-1, idx);

    idx = cmd_parameter_index(6, argv, "p1");
    TEST_ASSERT_EQUAL_INT(1, idx);
}
void test_param_2(void)
{
    char *argv[] =  { "cmd", "p1", "-p2", "false", "p4", "p5" };
    char *argv2[] = { "cmd", "p1", "-p2", "true",  "p4", "p5" };

    bool on, ok;
    ok = cmd_parameter_bool(6, argv, "-p2", &on);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_INT(false, on);

    ok = cmd_parameter_bool(6, argv2, "-p2", &on);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_INT(true, on);

    ok = cmd_parameter_bool(6, argv2, "p5", &on);
    TEST_ASSERT_EQUAL_INT(false, ok);
}
void test_param_3(void)
{
    bool ok;
    char *val;
    char *argv[] =  { "cmd", "p1", "p2", "p3", "p4", "p5" };

    ok = cmd_parameter_val(6, argv, "p2", &val);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_STRING("p3", val);

    ok = cmd_parameter_val(6, argv, "p3", &val);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_STRING("p4", val);

    ok = cmd_parameter_val(6, argv, "p5", &val);
    TEST_ASSERT_EQUAL_INT(false, ok);
}

void test_param_4(void)
{
    bool ok;
    int val;
    char *argv[] =  { "cmd", "p1", "p2", "3", "p4", "p5" };

    ok = cmd_parameter_int(6, argv, "p2", &val);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_INT(3, val);

    ok = cmd_parameter_int(6, argv, "p4", &val);
    TEST_ASSERT_EQUAL_INT(true, ok);
    TEST_ASSERT_EQUAL_INT(0, val);
}
void test_cmd_echo_1(void)
{
    REQUEST("echo Hi!");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("Hi! ") , buf);
}
void test_cmd_echo_1b(void)
{
    REQUEST(" echo Hi!");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("Hi! ") , buf);
}
void test_cmd_echo_2(void)
{
    REQUEST("echo foo faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo faa ") , buf);
}
void test_cmd_echo_3(void)
{
    REQUEST("echo foo   faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo faa ") , buf);
}
void test_cmd_echo_4(void)
{
    REQUEST("echo   foo   faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo faa ") , buf);
}
void test_cmd_echo_5(void)
{
    REQUEST("echo   \"foo   faa\"");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo   faa ") , buf);
}
void test_cmd_echo_6(void)
{
    REQUEST("echo   \"foo   faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("\"foo faa ") , buf);
}
void test_cmd_echo_7(void)
{
    REQUEST("echo   'foo   faa\"");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("'foo faa\" ") , buf);
}
void test_cmd_echo_8(void)
{
    REQUEST("echof\x7f foo   faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo faa ") , buf);
}
void test_cmd_echo_9(void)
{
    REQUEST("echo foo   faa\x1b[D\x1b[D\x1b[D hello ");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo hello faa ") , buf);
    CLEAN();
}
void test_cmd_echo_10(void)
{
    REQUEST("echo foo   faa\x1b[D\x1b[C\x1b[C  hello "); //echo foo    hello faa
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo faa hello ") , buf);
    CLEAN();
}
void test_cmd_echo_11(void)
{
    REQUEST("echo off\r");
    INIT_BUF();
    input("echo test");
    TEST_ASSERT_EQUAL_STRING("" , buf);
    input("\r");
    TEST_ASSERT_EQUAL_STRING("test \n" , buf);
    INIT_BUF();
    REQUEST("echo on\r");
    INIT_BUF();
    input("e");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("e ") , buf);
    INIT_BUF();
    input("c");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("ec ") , buf);
    INIT_BUF();
    input("h");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("ech ") , buf);
    INIT_BUF();
    input("o");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo ") , buf);
    INIT_BUF();
    input(" ");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo  ") , buf);
    INIT_BUF();
    input("o");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo o ") , buf);
    INIT_BUF();
    input("k");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo ok ") , buf);
    CLEAN();
}

void test_cmd_arrows_up(void)
{
    REQUEST("echo foo-1");
    INIT_BUF();
    input("\x1b[A");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo foo-1 ") , buf);
    INIT_BUF();
    input("\x1b[A");
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo foo-1 ") , buf);
    CLEAN();
}
void test_cmd_arrows_up_down(void)
{
    REQUEST("echo test-1");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("test-1 "), buf);
    REQUEST("echo test-2");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("test-2 "), buf);
    REQUEST("echo test-3");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("test-3 "), buf);

    INIT_BUF();
    UP();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    UP();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    UP();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    UP();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    DOWN();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    DOWN();
    TEST_ASSERT_EQUAL_STRING(CMDLINE(" "), buf);
    CLEAN();
}
void test_cmd_pageup_page_down(void)
{
    //goto history beginning/end
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    INIT_BUF();
    PAGE_UP();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-4 "), buf);
    CLEAN();
}
void test_cmd_text_pageup(void)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    TEST_ASSERT_EQUAL_STRING(CMDLINE("hello "), buf);
    CLEAN();
}
void test_cmd_text_pageup_up(void)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    TEST_ASSERT_EQUAL_STRING(CMDLINE("hello "), buf);
    CLEAN();
}
void test_cmd_text_delete(void)
{
    input("hello world");
    LEFT_N(2);
    DELETE();
    INIT_BUF();
    DELETE();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    TEST_ASSERT_EQUAL_STRING(CMDLINE("hello wor "), buf);
    LEFT_N(2);
    INIT_BUF();
    DELETE();
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("hello wr ", "2", BACKWARD), buf);
    BACKSPACE();
    BACKSPACE();
    INIT_BUF();
    BACKSPACE();
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("hellr ", "2", BACKWARD), buf);
    CLEAN();
}

void test_cmd_insert(void)
{
    CLEAN();
    input("echo hello word");
    LEFT();
    INIT_BUF();
    input("l");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo hello world ", "2", BACKWARD), buf);
    LEFT_N(10);
    INIT_BUF();
    LEFT();
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo hello world ", "13", BACKWARD), buf);
    INIT_BUF();
    RIGHT();
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo hello world ", "12", BACKWARD), buf);
    CLEAN();
}

void test_cmd_tab_1(void)
{
    INIT_BUF();
    input("e");
    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input("\rech");
    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input("\r");
}
void test_cmd_tab_2(void)
{
    INIT_BUF();

    cmd_add("role", cmd_dummy, 0, 0);
    cmd_add("route", cmd_dummy, 0, 0);
    cmd_add("rile", cmd_dummy, 0, 0);
    input("r");
    INIT_BUF();

    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("route ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("rile ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\x1b[Z");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("route ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\x1b[Z");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    input("\r");
}

void test_cmd_tab_3(void)
{
    INIT_BUF();
    cmd_add("role", cmd_dummy, 0, 0);
    cmd_alias_add("rose", "role");
    cmd_alias_add("rope", "rope");

    input("r");

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("rose ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("rope ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("r ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("o");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("ro ", "1", BACKWARD) , buf);

    ESC();
    INIT_BUF();
}

void test_cmd_tab_4(void)
{
    INIT_BUF();
    cmd_variable_add("dut1", "hello");

    input("e");

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input(" $d");
    INIT_BUF();
    input("u");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo $du ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo $dut1 ", "1", BACKWARD) , buf);

    input("\re");
    INIT_BUF();
    input("\t");
    TEST_ASSERT_EQUAL_STRING(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input("\r");
    INIT_BUF();
}

// alias test
extern void replace_alias(const char *str, const char *old, const char *new);
void test_cmd_alias_1(void)
{
    char str[] = "hello a men";
    replace_alias(str, "a", "b");
    TEST_ASSERT_EQUAL_STRING("hello a men", str);

    replace_alias(str, "hello", "echo");
    TEST_ASSERT_EQUAL_STRING("echo a men", str);
    INIT_BUF();
}
void test_cmd_alias_2(void)
{
    REQUEST("alias foo bar");
    INIT_BUF();
    REQUEST("alias");
    TEST_ASSERT_EQUAL_STRING("\nalias:\nfoo               'bar'\n\r\x1b[2K/> \x1b[1D", buf);

    REQUEST("alias foo");
    INIT_BUF();
    REQUEST("alias");
    TEST_ASSERT_EQUAL_STRING("\nalias:\n\r\x1b[2K/> \x1b[1D", buf);
}
void test_cmd_alias_3(void)
{
    cmd_alias_add("p", "echo");
    REQUEST("p toimii");
    TEST_ASSERT_EQUAL_STRING("\ntoimii \n\r\x1b[2K/> \x1b[1D", buf);

    cmd_alias_add("printtti", "echo");
    REQUEST("printtti toimii");
    TEST_ASSERT_EQUAL_STRING("\ntoimii \n\r\x1b[2K/> \x1b[1D", buf);
}
void test_cmd_alias_4(void)
{
    REQUEST("alias dut1 \"echo dut1\"");
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("dut1 "), buf);
}
void test_cmd_alias_5(void)
{
    REQUEST("alias dut1 \"echo dut1\"");
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1;dut2;dut3");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("dut1 \ndut2 \ndut3 "), buf);
}
void test_cmd_var_1(void)
{
    REQUEST("set foo \"bar test\"");
    INIT_BUF();
    REQUEST("set");
    TEST_ASSERT_EQUAL_STRING("\nvariables:\nfoo               'bar test'\n\r\x1b[2K/> \x1b[1D", buf);

    REQUEST("set foo");
    INIT_BUF();
    REQUEST("set");
    TEST_ASSERT_EQUAL_STRING("\nvariables:\n\r\x1b[2K/> \x1b[1D", buf);
}
void test_cmd_var_2(void)
{
    REQUEST("set foo \"hello world\"");
    REQUEST("echo foo");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("foo ") , buf);

    REQUEST("echo $foo");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("hello world ") , buf);

    REQUEST("set faa !");
    REQUEST("echo $foo$faa");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("hello world! ") , buf);
}
void test_multiple_cmd(void)
{
    REQUEST("set foo \"hello world\";echo $foo");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("hello world ") , buf);

    REQUEST("setd faa \"hello world\";echo $faa");
    TEST_ASSERT_EQUAL_STRING("\nCommand 'setd' not found.\n$faa \n\r\x1B[2K/> \x1B[1D" , buf);

    REQUEST("setd foo \"hello guy\"&&echo $foo");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("Command 'setd' not found.") , buf);
}
void test_maxlength(void)
{
    int i;
    char test_data[600];
    char *ptr = test_data;
    strcpy(test_data, "echo ");
    for (i = 5; i < 600; i++) {
        test_data[i] = 'A' + i % 26;
    }
    test_data[599] = 0;
    REQUEST(ptr);
    //TEST_ASSERT_EQUAL_STRING( RESPONSE((test_data+5)), buf);
}
void test_ampersand(void)
{
    REQUEST("echo hello world&");
    TEST_ASSERT_EQUAL_STRING(RESPONSE("hello world ") , buf);
}
