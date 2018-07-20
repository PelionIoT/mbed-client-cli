/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file \test\Test.c
 *
 * \brief Unit tests for mbed-client-cli
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "mbed-cpputest/CppUTest/TestHarness.h"
#include "mbed-cpputest/CppUTest/SimpleString.h"
#include "mbed-cpputest/CppUTest/CommandLineTestRunner.h"

#define MBED_CONF_MBED_TRACE_ENABLE 1
#define MBED_CONF_MBED_TRACE_FEA_IPV6 0
#include "mbed-trace/mbed_trace.h"
#include "mbed-client-cli/ns_cmdline.h"
#define MAX(x,y)   (x>y?x:y)
#define ARRAY_CMP(x, y) \
        MEMCMP_EQUAL(x, y, MAX(strlen(x), strlen(y)))

int main(int ac, char **av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}

#define BUFSIZE 1024
char buf[BUFSIZE] = {0};
#define INIT_BUF()  memset(buf, 0, BUFSIZE)
int cmd_dummy(int argc, char *argv[])
{
    return 0;
}

int mutex_wait_count = 0;
int mutex_release_count = 0;
int mutex_count_expected_difference = 1;
bool check_mutex_lock_state = false;
void my_mutex_wait()
{
    mutex_wait_count++;
}
void my_mutex_release()
{
    mutex_release_count++;
}

void myprint(const char *fmt, va_list ap)
{
    if (check_mutex_lock_state) {
        CHECK((mutex_wait_count - mutex_release_count) == mutex_count_expected_difference);
    }
    vsnprintf(buf + strlen(buf), BUFSIZE - strlen(buf), fmt, ap);
    //printf("\nMYPRINT: %s\n", buf); //for test test
}
void input(const char *str)
{
    while (*str != 0) {
        cmd_char_input(*str++);
    }
}

#define ESCAPE(x) "\x1b" x

#define DEFAULT_PROMPT      "/>"
#define FORWARD             "C"
#define BACKWARD            "D"
#define REQUEST(x)          input(x);INIT_BUF();cmd_char_input('\r');
#define PROMPT(input, prompt)    "\r" ESCAPE("[2K") prompt input ESCAPE("[1D")
#define RAW_RESPONSE_WITH_PROMPT(x, prompt) "\r\n" x PROMPT(" ", prompt)
#define RESPONSE_WITH_PROMPT(x, prompt) RAW_RESPONSE_WITH_PROMPT(x "\r\n", prompt)
#define RESPONSE(x)         RESPONSE_WITH_PROMPT(x, DEFAULT_PROMPT)

#define CMDLINE(x)          "\r" ESCAPE("[2K") "/>" x ESCAPE("[1D")
#define CMDLINE_EMPTY       CMDLINE(" ")
#define CMDLINE_CUR(x, cursor, dir)  "\r" ESCAPE("[2K") DEFAULT_PROMPT x ESCAPE("[" cursor dir)
#define CLEAN()             cmd_char_input('\r');INIT_BUF();

//vt100 keycodes
#define HOME()      input(ESCAPE("[1~"))
#define INSERT()    input(ESCAPE("[2~"))
#define DELETE()    input(ESCAPE("[3~"))
#define BACKSPACE() input("\x7f")
#define LEFT()      input(ESCAPE("[D"))
#define LEFT_N(n)   for(int i=0;i<n;i++) LEFT();
#define RIGHT()     input(ESCAPE("[C"))
#define RIGHT_N(n)  for(int i=0;i<n;i++) RIGHT()
#define UP()        input(ESCAPE("[A"))
#define DOWN()      input(ESCAPE("[B"))
#define ESC()       input("\x03")
#define PAGE_DOWN() input(ESCAPE("[6~"))
#define PAGE_UP()   input(ESCAPE("[5~"))

int previous_retcode = 0;
#define CHECK_RETCODE(retcode) CHECK_EQUAL(previous_retcode, retcode)
#define TEST_RETCODE_WITH_COMMAND(cmd, retcode) REQUEST(cmd);CHECK_RETCODE(retcode)

void cmd_ready_cb(int retcode)
{
    previous_retcode = retcode;
    cmd_next(retcode);
}

TEST_GROUP(cli)
{
  void setup()
  {
    cmd_init(&myprint);
    cmd_set_ready_cb(cmd_ready_cb);
    INIT_BUF();
  }
  void teardown()
  {
    INIT_BUF();
    cmd_free();
  }
};

TEST(cli, init)
{
}
TEST(cli, cmd_printf_with_mutex_not_set)
{
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
    int mutex_call_count_at_entry = mutex_wait_count;
    check_mutex_lock_state = false;

    cmd_printf("Hello hello!");
    STRCMP_EQUAL("Hello hello!" , buf);

    CHECK(mutex_call_count_at_entry == mutex_wait_count);
    CHECK(mutex_call_count_at_entry == mutex_release_count);

    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
}
TEST(cli, cmd_printf_with_mutex_set)
{
    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
    check_mutex_lock_state = true;

    cmd_printf("!olleh olleH");
    STRCMP_EQUAL("!olleh olleH" , buf);
    CHECK(mutex_wait_count == mutex_release_count);

    check_mutex_lock_state = false;
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
}
TEST(cli, external_mutex_handles)
{
    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
    check_mutex_lock_state = true;
    mutex_count_expected_difference = 2;

    cmd_mutex_lock();
    cmd_printf("!olleh olleH");
    STRCMP_EQUAL("!olleh olleH" , buf);
    cmd_mutex_unlock();
    CHECK(mutex_wait_count == mutex_release_count);

    mutex_count_expected_difference = 1;
    check_mutex_lock_state = false;
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
}
TEST(cli, parameters_index)
{
    char *argv[] = { "cmd", "p1", "p2", "p3", "p4", "p5" };
    int idx = cmd_parameter_index(6, argv, "p4");
    CHECK_EQUAL(4, idx);

    idx = cmd_parameter_index(6, argv, "p6");
    CHECK_EQUAL(-1, idx);

    idx = cmd_parameter_index(6, argv, "p1");
    CHECK_EQUAL(1, idx);
}

TEST(cli, parameters_bools)
{
    char *argv[] =  { "cmd", "p1", "-p2", "false", "p4", "p5" };
    char *argv2[] = { "cmd", "p1", "-p2", "true",  "p4", "p5" };

    bool on, ok;
    ok = cmd_parameter_bool(6, argv, "-p2", &on);
    CHECK_EQUAL(true, ok);
    CHECK_EQUAL(false, on);

    ok = cmd_parameter_bool(6, argv2, "-p2", &on);
    CHECK_EQUAL(true, ok);
    CHECK_EQUAL(true, on);

    ok = cmd_parameter_bool(6, argv2, "p5", &on);
    CHECK_EQUAL(false, ok);
}
TEST(cli, parameters_val)
{
    bool ok;
    char *val;
    char *argv[] =  { "cmd", "p1", "p2", "p3", "p4", "p5" };

    ok = cmd_parameter_val(6, argv, "p2", &val);
    CHECK_EQUAL(true, ok);
    ARRAY_CMP("p3", val);

    ok = cmd_parameter_val(6, argv, "p3", &val);
    CHECK_EQUAL(true, ok);
    ARRAY_CMP("p4", val);

    ok = cmd_parameter_val(6, argv, "p5", &val);
    CHECK_EQUAL(false, ok);
}

TEST(cli, parameters_int)
{
    bool ok;
    int val;
    char *argv[] =  { "cmd", "p1", "p2", "3", "p4", "555fail", "p5" };

    ok = cmd_parameter_int(6, argv, "p2", &val);
    CHECK_EQUAL(true, ok);
    CHECK_EQUAL(3, val);

    ok = cmd_parameter_int(6, argv, "p1", &val);
    CHECK_EQUAL(false, ok);

    ok = cmd_parameter_int(6, argv, "p4", &val);
    CHECK_EQUAL(false, ok);

    ok = cmd_parameter_int(6, argv, "p5", &val);
    CHECK_EQUAL(false, ok);
}
TEST(cli, parameters_float)
{
    bool ok;
    float val;
    float val2 = 3.14159;
    char *argv[] =  { "cmd", "p1", "3.14159", "p3", "3.14159 ", "p4", "3.14fail ", "p5" };

    ok = cmd_parameter_float(8, argv, "p1", &val);
    CHECK_EQUAL(true, ok);
    CHECK_EQUAL(val2, val);

    ok = cmd_parameter_float(8, argv, "p3", &val);
    CHECK_EQUAL(true, ok);
    CHECK_EQUAL(val2, val);

    ok = cmd_parameter_float(8, argv, "p4", &val);
    CHECK_EQUAL(false, ok);

    ok = cmd_parameter_float(8, argv, "p5", &val);
    CHECK_EQUAL(false, ok);
}
TEST(cli, cmd_parameter_last)
{
    char *argv[] =  { "cmd", "p1", "p2", "3", "p4", "p5" };
    CHECK_EQUAL(cmd_parameter_last(6, argv), "p5");
}
TEST(cli, cmd_has_option)
{
    char *argv[] =  { "cmd", "-p", "p2", "3", "p4", "p5" };
    CHECK_EQUAL(cmd_has_option(6, argv, "-p"), true);
}
TEST(cli, echo_state)
{
  CHECK_EQUAL(cmd_echo_state(), true);
  cmd_echo_off();
  CHECK_EQUAL(cmd_echo_state(), false);
  cmd_echo_on();
  CHECK_EQUAL(cmd_echo_state(), true);
}
TEST(cli, help)
{
    REQUEST("help");
    CHECK(strlen(buf) > 20 );
    CHECK_RETCODE(0);

    INIT_BUF();
    REQUEST("echo --help");
    CHECK(strlen(buf) > 20 );
    CHECK_RETCODE(0);
}
TEST(cli, retcodes)
{
    TEST_RETCODE_WITH_COMMAND("true", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("false", CMDLINE_RETCODE_FAIL);
    TEST_RETCODE_WITH_COMMAND("abc", CMDLINE_RETCODE_COMMAND_NOT_FOUND);
    TEST_RETCODE_WITH_COMMAND("set --abc", CMDLINE_RETCODE_INVALID_PARAMETERS);
}
TEST(cli, cmd_echo)
{
    REQUEST("echo Hi!");
    ARRAY_CMP(RESPONSE("Hi! ") , buf);
    CHECK_RETCODE(0);
}
TEST(cli, cmd_echo1)
{
    REQUEST(" echo Hi!");
    ARRAY_CMP(RESPONSE("Hi! ") , buf);
}
TEST(cli, cmd_echo2)
{
    REQUEST("echo foo faa");
    ARRAY_CMP(RESPONSE("foo faa ") , buf);
}
TEST(cli, cmd_echo3)
{
    REQUEST("echo foo   faa");
    ARRAY_CMP(RESPONSE("foo faa ") , buf);
}
TEST(cli, cmd_echo4)
{
    REQUEST("echo   foo   faa");
    ARRAY_CMP(RESPONSE("foo faa ") , buf);
}
TEST(cli, cmd_echo5)
{
    REQUEST("echo   \"foo   faa\"");
    ARRAY_CMP(RESPONSE("foo   faa ") , buf);
}
TEST(cli, cmd_echo6)
{
    REQUEST("echo   \"foo   faa");
    ARRAY_CMP(RESPONSE("\"foo faa ") , buf);
}
TEST(cli, cmd_echo7)
{
    REQUEST("echo   'foo   faa\"");
    ARRAY_CMP(RESPONSE("'foo faa\" ") , buf);
}
TEST(cli, cmd_echo8)
{
    REQUEST("echof\x7f foo   faa");
    ARRAY_CMP(RESPONSE("foo faa ") , buf);
}
TEST(cli, cmd_echo9)
{
    REQUEST("echo foo   faa\x1b[D\x1b[D\x1b[D hello ");
    ARRAY_CMP(RESPONSE("foo hello faa ") , buf);
    CLEAN();
}
TEST(cli, cmd_echo10)
{
    REQUEST("echo foo   faa\x1b[D\x1b[C\x1b[C  hello "); //echo foo    hello faa
    ARRAY_CMP(RESPONSE("foo faa hello ") , buf);
    CLEAN();
}
TEST(cli, cmd_echo11)
{
    REQUEST("echo off\r");
    INIT_BUF();
    input("echo test");
    ARRAY_CMP("" , buf);
    input("\r");
    ARRAY_CMP("test \r\n" , buf);
    INIT_BUF();
    REQUEST("echo on\r");
    INIT_BUF();
    input("e");
    ARRAY_CMP(CMDLINE("e ") , buf);
    INIT_BUF();
    input("c");
    ARRAY_CMP(CMDLINE("ec ") , buf);
    INIT_BUF();
    input("h");
    ARRAY_CMP(CMDLINE("ech ") , buf);
    INIT_BUF();
    input("o");
    ARRAY_CMP(CMDLINE("echo ") , buf);
    INIT_BUF();
    input(" ");
    ARRAY_CMP(CMDLINE("echo  ") , buf);
    INIT_BUF();
    input("o");
    ARRAY_CMP(CMDLINE("echo o ") , buf);
    INIT_BUF();
    input("k");
    ARRAY_CMP(CMDLINE("echo ok ") , buf);
    CLEAN();
}

TEST(cli, cmd_arrows_up)
{
    REQUEST("echo foo-1");
    INIT_BUF();
    input("\x1b[A");
    ARRAY_CMP(CMDLINE("echo foo-1 ") , buf);
    INIT_BUF();
    input("\x1b[A");
    ARRAY_CMP(CMDLINE("echo foo-1 ") , buf);
    CLEAN();
}
TEST(cli, cmd_arrows_up_down)
{
    REQUEST("echo test-1");
    ARRAY_CMP(RESPONSE("test-1 "), buf);
    REQUEST("echo test-2");
    ARRAY_CMP(RESPONSE("test-2 "), buf);
    REQUEST("echo test-3");
    ARRAY_CMP(RESPONSE("test-3 "), buf);

    INIT_BUF();
    UP();
    ARRAY_CMP(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    UP();
    ARRAY_CMP(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    UP();
    ARRAY_CMP(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    UP();
    ARRAY_CMP(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    ARRAY_CMP(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    DOWN();
    ARRAY_CMP(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    DOWN();
    ARRAY_CMP(CMDLINE(" "), buf);
    CLEAN();
}
TEST(cli, cmd_history)
{
    //history when there is some
    REQUEST("echo test");
    INIT_BUF();
    REQUEST("history");
    const char* to_be =
      "\r\nHistory [2/31]:\r\n" \
      "[0]: echo test\r\n" \
      "[1]: history\r\n" \
      CMDLINE_EMPTY;
    ARRAY_CMP(to_be, buf);
    CLEAN();
}
TEST(cli, cmd_history_skip_duplicates)
{
    //history when there is some
    REQUEST("echo test");
    REQUEST("echo test");
    REQUEST("echo test");
    INIT_BUF();
    REQUEST("history");
    const char* to_be =
      "\r\nHistory [2/31]:\r\n" \
      "[0]: echo test\r\n" \
      "[1]: history\r\n" \
      CMDLINE_EMPTY;
    ARRAY_CMP(to_be, buf);
    CLEAN();
}
TEST(cli, cmd_history_empty)
{
    //history when its empty
    INIT_BUF();
    REQUEST("history");
    const char* to_be =
      "\r\nHistory [1/31]:\r\n" \
      "[0]: history\r\n" \
      CMDLINE_EMPTY;
    ARRAY_CMP(to_be, buf);
    CLEAN();
}
TEST(cli, cmd_pageup_page_down)
{
    //goto history beginning/end
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    INIT_BUF();
    PAGE_UP();
    ARRAY_CMP(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN();
    ARRAY_CMP(CMDLINE("echo test-4 "), buf);
    CLEAN();
}
TEST(cli, cmd_text_pageup)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    ARRAY_CMP(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    ARRAY_CMP(CMDLINE("hello "), buf);
    CLEAN();
}
TEST(cli, cmd_text_pageup_up)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    ARRAY_CMP(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    ARRAY_CMP(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    ARRAY_CMP(CMDLINE("hello "), buf);
    CLEAN();
}
TEST(cli, cmd_text_delete)
{
    input("hello world");
    LEFT_N(2);
    DELETE();
    INIT_BUF();
    DELETE();
    ARRAY_CMP(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    ARRAY_CMP(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    ARRAY_CMP(CMDLINE("hello wor "), buf);
    LEFT_N(2);
    INIT_BUF();
    DELETE();
    ARRAY_CMP(CMDLINE_CUR("hello wr ", "2", BACKWARD), buf);
    BACKSPACE();
    BACKSPACE();
    INIT_BUF();
    BACKSPACE();
    ARRAY_CMP(CMDLINE_CUR("hellr ", "2", BACKWARD), buf);
    CLEAN();
}
TEST(cli, cmd_insert)
{
    CLEAN();
    input("echo hello word");
    LEFT();
    INIT_BUF();
    input("l");
    ARRAY_CMP(CMDLINE_CUR("echo hello world ", "2", BACKWARD), buf);
    LEFT_N(10);
    INIT_BUF();
    LEFT();
    ARRAY_CMP(CMDLINE_CUR("echo hello world ", "13", BACKWARD), buf);
    INIT_BUF();
    RIGHT();
    ARRAY_CMP(CMDLINE_CUR("echo hello world ", "12", BACKWARD), buf);
    CLEAN();
}
TEST(cli, ctrl_w)
{
  input("\r\n");
  input("echo ping pong");
  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR("echo ping  ", "1", BACKWARD), buf);

  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR("echo  ", "1", BACKWARD), buf);
  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR(" ", "1", BACKWARD), buf);
  CLEAN();
}

TEST(cli, ctrl_w_1)
{
  input("echo ping pong");
  LEFT();
  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR("echo ping g ", "2", BACKWARD), buf);
  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR("echo g ", "2", BACKWARD), buf);
  INIT_BUF();
  input("\x17");
  ARRAY_CMP(CMDLINE_CUR("g ", "2", BACKWARD), buf);
  CLEAN();
}
TEST(cli, cmd_request_screen_size)
{
  cmd_request_screen_size();
  input(ESCAPE("[6;7R"));
  INIT_BUF();
  REQUEST("set");
  ARRAY_CMP("\r\nvariables:\r\n"
            "PS1='/>'\r\n"
            "?=0\r\n"
            "LINES=6\r\n"
            "COLUMNS=7\r\n"
            CMDLINE_EMPTY, buf);
}

TEST(cli, cmd_tab_1)
{
    INIT_BUF();
    input("e");
    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input("\rech");
    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input("\r");
}
TEST(cli, cmd_tab_2)
{
    INIT_BUF();

    cmd_add("role", cmd_dummy, 0, 0);
    cmd_add("route", cmd_dummy, 0, 0);
    cmd_add("rile", cmd_dummy, 0, 0);
    input("r");
    INIT_BUF();

    input("\t");
    ARRAY_CMP(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("route ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("rile ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\x1b[Z");
    ARRAY_CMP(CMDLINE_CUR("route ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\x1b[Z");
    ARRAY_CMP(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    input("\r");
}
TEST(cli, cmd_delete)
{
    INIT_BUF();
    cmd_add("role", cmd_dummy, 0, 0);
    cmd_delete("role");
    REQUEST("role");
    CHECK_RETCODE(CMDLINE_RETCODE_COMMAND_NOT_FOUND);
}
TEST(cli, cmd_escape)
{
  INIT_BUF();
  REQUEST("echo \\\"");
  ARRAY_CMP(RESPONSE("\\\" "), buf);

  INIT_BUF();
  REQUEST("echo \"\\\\\"\"");
  ARRAY_CMP(RESPONSE("\\\" "), buf);
}
TEST(cli, cmd_tab_3)
{
    INIT_BUF();
    cmd_add("role", cmd_dummy, 0, 0);
    cmd_alias_add("rose", "role");
    cmd_alias_add("rope", "rope");

    input("r");

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("role ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("rose ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("rope ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("r ", "1", BACKWARD) , buf);

    INIT_BUF();
    input("o");
    ARRAY_CMP(CMDLINE_CUR("ro ", "1", BACKWARD) , buf);

    ESC();
    INIT_BUF();
}
TEST(cli, cmd_tab_4)
{
    INIT_BUF();
    cmd_variable_add("dut1", "hello");

    input("e");

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    input(" $d");
    INIT_BUF();
    input("u");
    ARRAY_CMP(CMDLINE_CUR("echo $du ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("echo $dut1 ", "1", BACKWARD) , buf);

    input("\re");
    INIT_BUF();
    input("\t");
    ARRAY_CMP(CMDLINE_CUR("echo ", "1", BACKWARD) , buf);

    cmd_variable_add("dut1", NULL);
    CLEAN();
}
// alias test
TEST(cli, cmd_alias_2)
{
    REQUEST("alias foo bar");
    INIT_BUF();
    REQUEST("alias");
    ARRAY_CMP("\r\nalias:\r\n"
              "foo               'bar'\r\n"
              "_                 'alias foo bar'\r\n"
              CMDLINE_EMPTY, buf);

    REQUEST("alias foo");
    INIT_BUF();
    REQUEST("alias");
    ARRAY_CMP("\r\nalias:\r\n"
              "_                 'alias foo'\r\n"
              CMDLINE_EMPTY, buf);
}
TEST(cli, cmd_alias_3)
{
    cmd_alias_add("p", "echo");
    REQUEST("p toimii");
    CHECK_RETCODE(0);
    ARRAY_CMP("\r\ntoimii \r\n" CMDLINE_EMPTY, buf);

    cmd_alias_add("printtti", "echo");
    REQUEST("printtti toimii");
    ARRAY_CMP("\r\ntoimii \r\n" CMDLINE_EMPTY, buf);
}
TEST(cli, cmd_alias_4)
{
    REQUEST("alias dut1 \"echo dut1\"");
    CHECK_RETCODE(0);
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1");
    CHECK_RETCODE(0);
    ARRAY_CMP(RESPONSE("dut1 "), buf);
}
TEST(cli, cmd_series)
{
    REQUEST("alias dut1 \"echo dut1\"");
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1;dut2;dut3");
    CHECK_RETCODE(0);
    ARRAY_CMP(RESPONSE("dut1 \r\ndut2 \r\ndut3 "), buf);
}

TEST(cli, cmd_var_1)
{
    REQUEST("set foo \"bar test\"");
    INIT_BUF();
    REQUEST("set");
    ARRAY_CMP("\r\nvariables:\r\n"
              "PS1='/>'\r\n"
              "?=0\r\n"
              "foo='bar test'\r\n"
              CMDLINE_EMPTY, buf);
    REQUEST("unset foo");
}
TEST(cli, cmd_unset)
{
    REQUEST("set foo=a");
    REQUEST("unset foo");
    INIT_BUF();
    REQUEST("set");
    ARRAY_CMP("\r\nvariables:\r\n"
              "PS1='/>'\r\n"
              "?=0\r\n"
              CMDLINE_EMPTY, buf);
}
TEST(cli, cmd_var_2)
{
    REQUEST("set foo \"hello world\"");
    CHECK_RETCODE(0);
    REQUEST("echo foo");
    ARRAY_CMP(RESPONSE("foo ") , buf);

    REQUEST("echo $foo");
    CHECK_RETCODE(0);
    ARRAY_CMP(RESPONSE("hello world ") , buf);

    REQUEST("set faa !");
    REQUEST("echo $foo$faa");
    ARRAY_CMP(RESPONSE("hello world! ") , buf);
    REQUEST("unset faa");
}
TEST(cli, cmd__)
{
    REQUEST("echo foo");
    ARRAY_CMP(RESPONSE("foo ") , buf);
    REQUEST("_");
    ARRAY_CMP(RESPONSE("foo ") , buf);
}
TEST(cli, var_prev_cmd)
{
    REQUEST("true");
    REQUEST("set");
    ARRAY_CMP("\r\nvariables:\r\n"
              "PS1='/>'\r\n"
              "?=0\r\n"
              CMDLINE_EMPTY, buf);
    REQUEST("false");
    REQUEST("set");
    ARRAY_CMP("\r\nvariables:\r\n"
              "PS1='/>'\r\n"
              "?=-1\r\n"
              CMDLINE_EMPTY, buf);
}
TEST(cli, var_ps1)
{
    REQUEST("set PS1=abc");
    ARRAY_CMP(RAW_RESPONSE_WITH_PROMPT("", "abc") , buf);
    REQUEST("set")
    ARRAY_CMP("\r\nvariables:\r\n"
              "PS1='abc'\r\n"
              "?=0\r\n"
              "\r" ESCAPE("[2K") "abc " ESCAPE("[1D"), buf);
}
// operators
TEST(cli, operator_semicolon)
{
    REQUEST("echo hello world")
    ARRAY_CMP(RESPONSE("hello world ") , buf);
    CHECK_RETCODE(CMDLINE_RETCODE_SUCCESS);

    REQUEST("setd faa \"hello world\";echo $faa");
    ARRAY_CMP("\r\nCommand 'setd' not found.\r\n$faa \r\n" CMDLINE_EMPTY , buf);
}
TEST(cli, operators_and)
{
  TEST_RETCODE_WITH_COMMAND("true && true", CMDLINE_RETCODE_SUCCESS);
  TEST_RETCODE_WITH_COMMAND("true && false", CMDLINE_RETCODE_FAIL);
  TEST_RETCODE_WITH_COMMAND("false && true", CMDLINE_RETCODE_FAIL);
  TEST_RETCODE_WITH_COMMAND("false && false", CMDLINE_RETCODE_FAIL);
}
TEST(cli, operators_or)
{
  TEST_RETCODE_WITH_COMMAND("true || true", CMDLINE_RETCODE_SUCCESS);
  TEST_RETCODE_WITH_COMMAND("true || false", CMDLINE_RETCODE_SUCCESS);
  TEST_RETCODE_WITH_COMMAND("false || true", CMDLINE_RETCODE_SUCCESS);
  TEST_RETCODE_WITH_COMMAND("false || false", CMDLINE_RETCODE_FAIL);
}
TEST(cli, ampersand)
{
    REQUEST("echo hello world&");
    ARRAY_CMP(RESPONSE("hello world ") , buf);
}
TEST(cli, maxlength)
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
    //ARRAY_CMP( RESPONSE((test_data+5)), buf);
}

#define REDIR_DATA "echo Hi!"
#define PASSTHROUGH_BUF_LENGTH 10
char passthrough_buffer[PASSTHROUGH_BUF_LENGTH];
char* passthrough_ptr = NULL;
void passthrough_cb(uint8_t c)
{
    if (passthrough_ptr != NULL) {
        *passthrough_ptr++ = c;
    }
}
TEST(cli, passthrough_set)
{
    passthrough_ptr = passthrough_buffer;
    memset(&passthrough_buffer, 0, PASSTHROUGH_BUF_LENGTH);
    INIT_BUF();

    cmd_input_passthrough_func(passthrough_cb);
    input(REDIR_DATA);

    CHECK(strlen(buf) == 0);
    ARRAY_CMP(REDIR_DATA, passthrough_buffer);

    cmd_input_passthrough_func(NULL);

    REQUEST(REDIR_DATA);
    ARRAY_CMP(RESPONSE("Hi! ") , buf);
}

int cmd_long_called = 0;
int cmd_long(int argc, char* argv[])
{
  cmd_long_called ++;
  return CMDLINE_RETCODE_EXCUTING_CONTINUE;
}
TEST(cli, cmd_continue)
{
  cmd_add("long", cmd_long, 0, 0);
  previous_retcode = 111;
  TEST_RETCODE_WITH_COMMAND("long", previous_retcode);
  cmd_ready(0);
  CHECK_RETCODE(CMDLINE_RETCODE_SUCCESS);
  CHECK_EQUAL(cmd_long_called, 1);
}

TEST(cli, cmd_out_func_set_null)
{
    cmd_out_func(NULL);
}

static int outf_called = 0;
void outf(const char *fmt, va_list ap) {
    outf_called++;
}
TEST(cli, cmd_out_func_set)
{
    outf_called = 0;
    cmd_out_func(&outf);
    // cppcheck-suppress formatExtraArgs
    cmd_vprintf(NULL, NULL);
    CHECK_EQUAL(outf_called, 1);
}

TEST(cli, cmd_ctrl_func_set_null)
{
    cmd_ctrl_func(NULL);
}

int sohf_cb_called = 0;
void sohf_cb(uint8_t c) {
  sohf_cb_called++;
}
TEST(cli, cmd_ctrl_func_set)
{
    cmd_ctrl_func(sohf_cb);
    REQUEST("\x04");
    CHECK_EQUAL(sohf_cb_called, 1);
    cmd_ctrl_func(NULL);
}

TEST(cli, cmd_delete_null)
{
    cmd_delete(NULL);
}

TEST(cli, cmd_history_size_set)
{
    cmd_history_size(0);
    CHECK_EQUAL(cmd_history_size(1), 1);
}

TEST(cli, cmd_add_invalid_params)
{
    cmd_add(NULL, cmd_dummy, NULL, NULL);
    cmd_add("", cmd_dummy, NULL, NULL);
    cmd_add("abc", NULL, NULL, NULL);
}
