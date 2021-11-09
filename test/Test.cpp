/*
 * Copyright (c) 2015-2019, Pelion and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
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

#include "gtest/gtest.h"

#define MBED_CONF_MBED_TRACE_ENABLE 1
#define MBED_CONF_MBED_TRACE_FEA_IPV6 0

/*
#define MBED_CONF_CMDLINE_ENABLE_HISTORY 1
#define MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING 1
#define MBED_CONF_CMDLINE_ENABLE_OPERATORS 1
#define MBED_CONF_CMDLINE_ENABLE_ALIASES 1
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS 1
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_VARIABLES 1
#define MBED_CONF_CMDLINE_INCLUDE_MAN 1
#define MBED_CONF_CMDLINE_MAX_LINE_LENGTH 100
#define MBED_CONF_CMDLINE_ARGUMENTS_MAX_COUNT 2
#define MBED_CONF_CMDLINE_HISTORY_MAX_COUNT 1
*/

#if MBED_CONF_CMDLINE_USE_MINIMUM_SET == 1
// this is copypaste from pre-defined minimum config
#define MBED_CONF_CMDLINE_INIT_AUTOMATION_MODE 1
#define MBED_CONF_CMDLINE_ENABLE_HISTORY 0
#define MBED_CONF_CMDLINE_ENABLE_ALIASES 0
#define MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING 0
#define MBED_CONF_CMDLINE_ENABLE_OPERATORS 0
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS 0
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_VARIABLES 0
#define MBED_CONF_CMDLINE_INCLUDE_MAN 0
#define MBED_CONF_CMDLINE_MAX_LINE_LENGTH 100
#define MBED_CONF_CMDLINE_ARGUMENTS_MAX_COUNT 10
#define MBED_CONF_CMDLINE_HISTORY_MAX_COUNT 0
#else
// this is copypaste from pre-defined minimum config
#define MBED_CONF_CMDLINE_INIT_AUTOMATION_MODE 0
#define MBED_CONF_CMDLINE_ENABLE_HISTORY 1
#define MBED_CONF_CMDLINE_ENABLE_ALIASES 1
#define MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING 1
#define MBED_CONF_CMDLINE_ENABLE_OPERATORS 1
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS 1
#define MBED_CONF_CMDLINE_ENABLE_INTERNAL_VARIABLES 1
#define MBED_CONF_CMDLINE_INCLUDE_MAN 1
#define MBED_CONF_CMDLINE_MAX_LINE_LENGTH 2000
#define MBED_CONF_CMDLINE_ARGUMENTS_MAX_COUNT 30
#define MBED_CONF_CMDLINE_HISTORY_MAX_COUNT 10
#endif

#include "mbed-trace/mbed_trace.h"
#include "mbed-client-cli/ns_cmdline.h"
#define MAX(x,y)   (x>y?x:y)

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
        ASSERT_TRUE((mutex_wait_count - mutex_release_count) == mutex_count_expected_difference);
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
#define LF                  '\n'
#define LF_S                "\n"
#define CR                  '\r'
#define CR_S                "\r"
#define DEFAULT_PROMPT      "/>"
#define FORWARD             "C"
#define BACKWARD            "D"
#define REQUEST(x)          input(x);INIT_BUF();cmd_char_input(LF);
#define PROMPT(input, prompt)    CR_S ESCAPE("[2K") prompt input ESCAPE("[1D")
#define RAW_RESPONSE_WITH_PROMPT(x, prompt) CR_S LF_S x PROMPT(" ", prompt)
#define RESPONSE_WITH_PROMPT(x, prompt) RAW_RESPONSE_WITH_PROMPT(x CR_S LF_S, prompt)
#define RESPONSE(x)         RESPONSE_WITH_PROMPT(x, DEFAULT_PROMPT)

#define CMDLINE(x)          CR_S ESCAPE("[2K") DEFAULT_PROMPT x ESCAPE("[1D")
#define CMDLINE_EMPTY       CMDLINE(" ")
#define CMDLINE_CUR(x, cursor, dir)  CR_S ESCAPE("[2K") DEFAULT_PROMPT x ESCAPE("[" cursor dir)
#define CLEAN()             cmd_char_input(LF);INIT_BUF();

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
#define ALT_LEFT()  input(ESCAPE("[b"))
#define ALT_RIGHT()  input(ESCAPE("[f"))

int previous_retcode = 0;
#define CHECK_RETCODE(retcode) EXPECT_EQ(previous_retcode, retcode)
#define TEST_RETCODE_WITH_COMMAND(cmd, retcode) REQUEST(cmd);CHECK_RETCODE(retcode)

void cmd_ready_cb(int retcode)
{
    previous_retcode = retcode;
    cmd_next(retcode);
}

class mbedClientCli : public testing::Test {
protected:
    void SetUp()
    {
        cmd_init(&myprint);
        cmd_set_ready_cb(cmd_ready_cb);
        INIT_BUF();
    }

    void TearDown()
    {
        INIT_BUF();
        cmd_free();
    }
};

TEST_F(mbedClientCli, init)
{
}
TEST_F(mbedClientCli, cmd_printf_with_mutex_not_set)
{
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
    int mutex_call_count_at_entry = mutex_wait_count;
    check_mutex_lock_state = false;

    cmd_printf("Hello hello!");
    EXPECT_STREQ("Hello hello!", buf);

    ASSERT_TRUE(mutex_call_count_at_entry == mutex_wait_count);
    ASSERT_TRUE(mutex_call_count_at_entry == mutex_release_count);

    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
}
TEST_F(mbedClientCli, cmd_printf_with_mutex_set)
{
    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
    check_mutex_lock_state = true;

    cmd_printf("!olleh olleH");
    EXPECT_STREQ("!olleh olleH", buf);
    ASSERT_TRUE(mutex_wait_count == mutex_release_count);

    check_mutex_lock_state = false;
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
}
TEST_F(mbedClientCli, external_mutex_handles)
{
    cmd_mutex_wait_func(my_mutex_wait);
    cmd_mutex_release_func(my_mutex_release);
    check_mutex_lock_state = true;
    mutex_count_expected_difference = 2;

    cmd_mutex_lock();
    cmd_printf("!olleh olleH");
    EXPECT_STREQ("!olleh olleH", buf);
    cmd_mutex_unlock();
    ASSERT_TRUE(mutex_wait_count == mutex_release_count);

    mutex_count_expected_difference = 1;
    check_mutex_lock_state = false;
    cmd_mutex_wait_func(0);
    cmd_mutex_release_func(0);
}
TEST_F(mbedClientCli, parameters_index)
{
    char *argv[] = { "cmd", "p1", "p2", "p3", "p4", "p5" };
    int idx = cmd_parameter_index(6, argv, "p4");
    EXPECT_EQ(4, idx);

    idx = cmd_parameter_index(6, argv, "p6");
    EXPECT_EQ(-1, idx);

    idx = cmd_parameter_index(6, argv, "p1");
    EXPECT_EQ(1, idx);
}

TEST_F(mbedClientCli, parameters_bools)
{
    char *argv[] =  { "cmd", "p1", "-p2", "false", "p4", "p5" };
    char *argv2[] = { "cmd", "p1", "-p2", "true",  "p4", "p5" };

    bool on, ok;
    ok = cmd_parameter_bool(6, argv, "-p2", &on);
    EXPECT_EQ(true, ok);
    EXPECT_EQ(false, on);

    ok = cmd_parameter_bool(6, argv2, "-p2", &on);
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, on);

    ok = cmd_parameter_bool(6, argv2, "p5", &on);
    EXPECT_EQ(false, ok);
}
TEST_F(mbedClientCli, parameters_val)
{
    bool ok;
    char *val;
    char *argv[] =  { "cmd", "p1", "p2", "p3", "p4", "p5" };

    ok = cmd_parameter_val(6, argv, "p2", &val);
    EXPECT_EQ(true, ok);
    EXPECT_STREQ("p3", val);

    ok = cmd_parameter_val(6, argv, "p3", &val);
    EXPECT_EQ(true, ok);
    EXPECT_STREQ("p4", val);

    ok = cmd_parameter_val(6, argv, "p5", &val);
    EXPECT_EQ(false, ok);
}

TEST_F(mbedClientCli, parameters_int)
{
    bool ok;
    int val;
    char *argv[] =  { "cmd", "p1", "p2", "3", "p4", "555fail", "p5" };

    ok = cmd_parameter_int(6, argv, "p2", &val);
    EXPECT_EQ(true, ok);
    EXPECT_EQ(3, val);

    ok = cmd_parameter_int(6, argv, "p1", &val);
    EXPECT_EQ(false, ok);

    ok = cmd_parameter_int(6, argv, "p4", &val);
    EXPECT_EQ(false, ok);

    ok = cmd_parameter_int(6, argv, "p5", &val);
    EXPECT_EQ(false, ok);
}
TEST_F(mbedClientCli, parameters_float)
{
    bool ok;
    float val;
    float val2 = 3.14159;
    char *argv[] =  { "cmd", "p1", "3.14159", "p3", "3.14159 ", "p4", "3.14fail ", "p5" };

    ok = cmd_parameter_float(8, argv, "p1", &val);
    EXPECT_EQ(true, ok);
    EXPECT_EQ(val2, val);

    ok = cmd_parameter_float(8, argv, "p3", &val);
    EXPECT_EQ(true, ok);
    EXPECT_EQ(val2, val);

    ok = cmd_parameter_float(8, argv, "p4", &val);
    EXPECT_EQ(false, ok);

    ok = cmd_parameter_float(8, argv, "p5", &val);
    EXPECT_EQ(false, ok);
}
TEST_F(mbedClientCli, cmd_parameter_last)
{
    char *argv[] =  { "cmd", "p1", "p2", "3", "p4", "p5" };
    EXPECT_EQ(cmd_parameter_last(6, argv), "p5");
}
TEST_F(mbedClientCli, cmd_parameter_last_not_exists)
{
    char *argv[] =  { "cmd" };
//    EXPECT_EQ(cmd_parameter_last(1, argv), NULL);
}
TEST_F(mbedClientCli, cmd_has_option)
{
    char *argv[] =  { "cmd", "-p", "p2", "3", "p4", "p5" };
    EXPECT_EQ(cmd_has_option(6, argv, "-p"), true);
}
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS == 1
TEST_F(mbedClientCli, echo_state)
{
    EXPECT_EQ(cmd_echo_state(), true);
    cmd_echo_off();
    EXPECT_EQ(cmd_echo_state(), false);
    cmd_echo_on();
    EXPECT_EQ(cmd_echo_state(), true);
}
#endif

TEST_F(mbedClientCli, help)
{
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS == 0
    TEST_RETCODE_WITH_COMMAND("help", CMDLINE_RETCODE_COMMAND_NOT_FOUND);
#else
    REQUEST("help");
    ASSERT_TRUE(strlen(buf) > 20);
    CHECK_RETCODE(0);

    INIT_BUF();
    REQUEST("echo --help");
    ASSERT_TRUE(strlen(buf) > 20);
    CHECK_RETCODE(0);
#endif
}

TEST_F(mbedClientCli, retcodes)
{
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS
    TEST_RETCODE_WITH_COMMAND("true", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("false", CMDLINE_RETCODE_FAIL);
    TEST_RETCODE_WITH_COMMAND("set --abc", CMDLINE_RETCODE_INVALID_PARAMETERS);
#endif
    TEST_RETCODE_WITH_COMMAND("abc", CMDLINE_RETCODE_COMMAND_NOT_FOUND);
}
TEST_F(mbedClientCli, cmd_echo)
{
#if MBED_CONF_CMDLINE_INIT_AUTOMATION_MODE == 1
    TEST_RETCODE_WITH_COMMAND("echo Hi!", CMDLINE_RETCODE_SUCCESS);
#else
    REQUEST("echo Hi!");
    EXPECT_STREQ(RESPONSE("Hi! "), buf);
    CHECK_RETCODE(0);
#endif
}
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS
TEST_F(mbedClientCli, cmd_echo_with_cr)
{
    input("echo crlf");
    INIT_BUF();
    input("\r\n");
    EXPECT_STREQ(RESPONSE("crlf "), buf);
    CHECK_RETCODE(0);
}
TEST_F(mbedClientCli, cmd_echo_cr_only)
{
    input("echo cr");
    INIT_BUF();
    input("\r");
    EXPECT_STREQ(RESPONSE("cr "), buf);
    CHECK_RETCODE(0);
}
TEST_F(mbedClientCli, cmd_echo1)
{
    REQUEST(" echo Hi!");
    EXPECT_STREQ(RESPONSE("Hi! "), buf);
}
TEST_F(mbedClientCli, cmd_echo2)
{
    REQUEST("echo foo faa");
    EXPECT_STREQ(RESPONSE("foo faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo3)
{
    REQUEST("echo foo   faa");
    EXPECT_STREQ(RESPONSE("foo faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo4)
{
    REQUEST("echo   foo   faa");
    EXPECT_STREQ(RESPONSE("foo faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo5)
{
    REQUEST("echo   \"foo   faa\"");
    EXPECT_STREQ(RESPONSE("foo   faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo6)
{
    REQUEST("echo   \"foo   faa");
    EXPECT_STREQ(RESPONSE("\"foo faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo7)
{
    REQUEST("echo   'foo   faa\"");
    EXPECT_STREQ(RESPONSE("'foo faa\" "), buf);
}
TEST_F(mbedClientCli, cmd_echo8)
{
    REQUEST("echof\x7f foo   faa");
    EXPECT_STREQ(RESPONSE("foo faa "), buf);
}
TEST_F(mbedClientCli, cmd_echo9)
{
    REQUEST("echo foo   faa\x1b[D\x1b[D\x1b[D hello ");
    EXPECT_STREQ(RESPONSE("foo hello faa "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_echo10)
{
    REQUEST("echo foo   faa\x1b[D\x1b[C\x1b[C  hello "); //echo foo    hello faa
    EXPECT_STREQ(RESPONSE("foo faa hello "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_echo11)
{
    REQUEST("echo off\n");
    INIT_BUF();
    input("echo test");
    EXPECT_STREQ("", buf);
    input("\n");
    EXPECT_STREQ("test \r\n", buf);
    INIT_BUF();
    REQUEST("echo on\n");
    INIT_BUF();
    input("e");
    EXPECT_STREQ(CMDLINE("e "), buf);
    INIT_BUF();
    input("c");
    EXPECT_STREQ(CMDLINE("ec "), buf);
    INIT_BUF();
    input("h");
    EXPECT_STREQ(CMDLINE("ech "), buf);
    INIT_BUF();
    input("o");
    EXPECT_STREQ(CMDLINE("echo "), buf);
    INIT_BUF();
    input(" ");
    EXPECT_STREQ(CMDLINE("echo  "), buf);
    INIT_BUF();
    input("o");
    EXPECT_STREQ(CMDLINE("echo o "), buf);
    INIT_BUF();
    input("k");
    EXPECT_STREQ(CMDLINE("echo ok "), buf);
    CLEAN();
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_HISTORY
TEST_F(mbedClientCli, cmd_arrows_up)
{
    REQUEST("echo foo-1");
    INIT_BUF();
    input("\x1b[A");
    EXPECT_STREQ(CMDLINE("echo foo-1 "), buf);
    INIT_BUF();
    input("\x1b[A");
    EXPECT_STREQ(CMDLINE("echo foo-1 "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_arrows_up_down)
{
    REQUEST("echo test-1");
    EXPECT_STREQ(RESPONSE("test-1 "), buf);
    REQUEST("echo test-2");
    EXPECT_STREQ(RESPONSE("test-2 "), buf);
    REQUEST("echo test-3");
    EXPECT_STREQ(RESPONSE("test-3 "), buf);

    INIT_BUF();
    UP();
    EXPECT_STREQ(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    UP();
    EXPECT_STREQ(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    UP();
    EXPECT_STREQ(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    UP();
    EXPECT_STREQ(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    EXPECT_STREQ(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    DOWN();
    EXPECT_STREQ(CMDLINE("echo test-3 "), buf);
    INIT_BUF();
    DOWN();
    EXPECT_STREQ(CMDLINE(" "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_set)
{
    TEST_RETCODE_WITH_COMMAND("set abc def", CMDLINE_RETCODE_SUCCESS);
#if MBED_CONF_CMDLINE_INIT_AUTOMATION_MODE == 0 && MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS == 1
    REQUEST("echo $abc");
    EXPECT_STREQ(RESPONSE("def "), buf);
#endif
}
TEST_F(mbedClientCli, cmd_history)
{
    //history when there is some
    REQUEST("echo test");
    INIT_BUF();
    REQUEST("history");
    const char *to_be =
        "\r\nHistory [2/31]:\r\n" \
        "[0]: echo test\r\n" \
        "[1]: history\r\n" \
        CMDLINE_EMPTY;
    EXPECT_STREQ(to_be, buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_history_clear)
{
    //history when there is some
    REQUEST("echo test");
    TEST_RETCODE_WITH_COMMAND("history clear", CMDLINE_RETCODE_SUCCESS);
    INIT_BUF();
    REQUEST("history");
    const char *to_be =
        "\r\nHistory [1/31]:\r\n" \
        "[0]: history\r\n" \
        CMDLINE_EMPTY;
    EXPECT_STREQ(to_be, buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_history_skip_duplicates)
{
    //history when there is some
    REQUEST("echo test");
    REQUEST("echo test");
    REQUEST("echo test");
    INIT_BUF();
    REQUEST("history");
    const char *to_be =
        "\r\nHistory [2/31]:\r\n" \
        "[0]: echo test\r\n" \
        "[1]: history\r\n" \
        CMDLINE_EMPTY;
    EXPECT_STREQ(to_be, buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_history_empty)
{
    //history when its empty
    INIT_BUF();
    REQUEST("history");
    const char *to_be =
        "\r\nHistory [1/31]:\r\n" \
        "[0]: history\r\n" \
        CMDLINE_EMPTY;
    EXPECT_STREQ(to_be, buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_pageup_page_down)
{
    //goto history beginning/end
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    INIT_BUF();
    PAGE_UP();
    EXPECT_STREQ(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN();
    EXPECT_STREQ(CMDLINE("echo test-4 "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_text_pageup)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    EXPECT_STREQ(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    EXPECT_STREQ(CMDLINE("hello "), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_text_pageup_up)
{
    REQUEST("echo test-1");
    REQUEST("echo test-2");
    REQUEST("echo test-3");
    REQUEST("echo test-4");
    input("hello");
    INIT_BUF();
    PAGE_UP(); //goto end of history
    EXPECT_STREQ(CMDLINE("echo test-1 "), buf);
    INIT_BUF();
    DOWN();
    EXPECT_STREQ(CMDLINE("echo test-2 "), buf);
    INIT_BUF();
    PAGE_DOWN(); //goto beginning of history - it should be just writted "hello"
    EXPECT_STREQ(CMDLINE("hello "), buf);
    CLEAN();
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING
TEST_F(mbedClientCli, cmd_alt_left_right)
{
    input("11 22 33");
    INIT_BUF();
    ALT_LEFT();
    EXPECT_STREQ(CMDLINE_CUR("11 22 33 ", "3", BACKWARD), buf);
    INIT_BUF();
    ALT_LEFT();
    EXPECT_STREQ(CMDLINE_CUR("11 22 33 ", "6", BACKWARD), buf);
    INIT_BUF();
    ALT_LEFT();
    EXPECT_STREQ(CMDLINE_CUR("11 22 33 ", "9", BACKWARD), buf);
    INIT_BUF();
    input("a");
    EXPECT_STREQ(CMDLINE_CUR("a11 22 33 ", "9", BACKWARD), buf);
    INIT_BUF();
    ALT_RIGHT();
    EXPECT_STREQ(CMDLINE_CUR("a11 22 33 ", "7", BACKWARD), buf);
    INIT_BUF();
    ALT_RIGHT();
    EXPECT_STREQ(CMDLINE_CUR("a11 22 33 ", "4", BACKWARD), buf);
    INIT_BUF();
    ALT_RIGHT();
    EXPECT_STREQ(CMDLINE_CUR("a11 22 33 ", "1", BACKWARD), buf);
    INIT_BUF();
    input("a");
    EXPECT_STREQ(CMDLINE_CUR("a11 22 33a ", "1", BACKWARD), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_text_delete)
{
    input("hello world");
    LEFT_N(2);
    DELETE();
    INIT_BUF();
    DELETE();
    EXPECT_STREQ(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    EXPECT_STREQ(CMDLINE("hello wor "), buf);
    INIT_BUF();
    DELETE();
    EXPECT_STREQ(CMDLINE("hello wor "), buf);
    LEFT_N(2);
    INIT_BUF();
    DELETE();
    EXPECT_STREQ(CMDLINE_CUR("hello wr ", "2", BACKWARD), buf);
    BACKSPACE();
    BACKSPACE();
    INIT_BUF();
    BACKSPACE();
    EXPECT_STREQ(CMDLINE_CUR("hellr ", "2", BACKWARD), buf);
    CLEAN();
}
TEST_F(mbedClientCli, cmd_insert)
{
    CLEAN();
    input("echo hello word");
    LEFT();
    INIT_BUF();
    input("l");
    EXPECT_STREQ(CMDLINE_CUR("echo hello world ", "2", BACKWARD), buf);
    LEFT_N(10);
    INIT_BUF();
    LEFT();
    EXPECT_STREQ(CMDLINE_CUR("echo hello world ", "13", BACKWARD), buf);
    INIT_BUF();
    RIGHT();
    EXPECT_STREQ(CMDLINE_CUR("echo hello world ", "12", BACKWARD), buf);
    CLEAN();
}
TEST_F(mbedClientCli, ctrl_w)
{
    input("\r\n");
    input("echo ping pong");
    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR("echo ping  ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR("echo  ", "1", BACKWARD), buf);
    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR(" ", "1", BACKWARD), buf);
    CLEAN();
}

TEST_F(mbedClientCli, ctrl_w_1)
{
    input("echo ping pong");
    LEFT();
    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR("echo ping g ", "2", BACKWARD), buf);
    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR("echo g ", "2", BACKWARD), buf);
    INIT_BUF();
    input("\x17");
    EXPECT_STREQ(CMDLINE_CUR("g ", "2", BACKWARD), buf);
    CLEAN();
}
#endif

#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_VARIABLES
TEST_F(mbedClientCli, cmd_request_screen_size)
{
    cmd_request_screen_size();
    input(ESCAPE("[6;7R"));
    INIT_BUF();
    REQUEST("set");
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='/>'\r\n"
                 "?=0\r\n"
                 "LINES=6\r\n"
                 "COLUMNS=7\r\n"
                 CMDLINE_EMPTY, buf);
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING
TEST_F(mbedClientCli, cmd_tab_1)
{
    INIT_BUF();
    input("e");
    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("echo ", "1", BACKWARD), buf);

    input("\nech");
    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("echo ", "1", BACKWARD), buf);

    input("\n");
}
TEST_F(mbedClientCli, cmd_tab_2)
{
    INIT_BUF();

    cmd_add("role", cmd_dummy, 0, 0);
    cmd_add("route", cmd_dummy, 0, 0);
    cmd_add("rile", cmd_dummy, 0, 0);
    input("r");
    INIT_BUF();

    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("role ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("route ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("rile ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\x1b[Z");
    EXPECT_STREQ(CMDLINE_CUR("route ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\x1b[Z");
    EXPECT_STREQ(CMDLINE_CUR("role ", "1", BACKWARD), buf);

    input("\n");
}
#endif
TEST_F(mbedClientCli, cmd_delete)
{
    INIT_BUF();
    cmd_add("role", cmd_dummy, 0, 0);
    cmd_delete("role");
    REQUEST("role");
    CHECK_RETCODE(CMDLINE_RETCODE_COMMAND_NOT_FOUND);
}
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS == 1
TEST_F(mbedClientCli, cmd_escape)
{
    INIT_BUF();
    REQUEST("echo \\\"");
    EXPECT_STREQ(RESPONSE("\\\" "), buf);

    INIT_BUF();
    REQUEST("echo \"\\\\\"\"");
    EXPECT_STREQ(RESPONSE("\\\" "), buf);
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_ALIASES == 1
TEST_F(mbedClientCli, cmd_tab_3)
{
    INIT_BUF();
    cmd_add("role", cmd_dummy, 0, 0);
    cmd_alias_add("rose", "role");
    cmd_alias_add("rope", "rope");

    input("r");

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("role ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("rose ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("rope ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("r ", "1", BACKWARD), buf);

    INIT_BUF();
    input("o");
    EXPECT_STREQ(CMDLINE_CUR("ro ", "1", BACKWARD), buf);

    ESC();
    INIT_BUF();
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS == 1
TEST_F(mbedClientCli, cmd_tab_4)
{
    INIT_BUF();
    cmd_variable_add("dut1", "hello");

    input("e");

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("echo ", "1", BACKWARD), buf);

    input(" $d");
    INIT_BUF();
    input("u");
    EXPECT_STREQ(CMDLINE_CUR("echo $du ", "1", BACKWARD), buf);

    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("echo $dut1 ", "1", BACKWARD), buf);

    input("\ne");
    INIT_BUF();
    input("\t");
    EXPECT_STREQ(CMDLINE_CUR("echo ", "1", BACKWARD), buf);

    cmd_variable_add("dut1", NULL);
    CLEAN();
}

// alias test
TEST_F(mbedClientCli, cmd_alias_2)
{
    REQUEST("alias foo bar");
    INIT_BUF();
    REQUEST("alias");
    EXPECT_STREQ("\r\nalias:\r\n"
                 "foo               'bar'\r\n"
                 "_                 'alias foo bar'\r\n"
                 CMDLINE_EMPTY, buf);

    REQUEST("alias foo");
    INIT_BUF();
    REQUEST("alias");
    EXPECT_STREQ("\r\nalias:\r\n"
                 "_                 'alias foo'\r\n"
                 CMDLINE_EMPTY, buf);
}
TEST_F(mbedClientCli, cmd_alias_3)
{
    cmd_alias_add("p", "echo");
    REQUEST("p toimii");
    CHECK_RETCODE(0);
    EXPECT_STREQ("\r\ntoimii \r\n" CMDLINE_EMPTY, buf);

    cmd_alias_add("printtti", "echo");
    REQUEST("printtti toimii");
    EXPECT_STREQ("\r\ntoimii \r\n" CMDLINE_EMPTY, buf);
}
TEST_F(mbedClientCli, cmd_alias_4)
{
    REQUEST("alias dut1 \"echo dut1\"");
    CHECK_RETCODE(0);
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1");
    CHECK_RETCODE(0);
    EXPECT_STREQ(RESPONSE("dut1 "), buf);
}
TEST_F(mbedClientCli, cmd_series)
{
    REQUEST("alias dut1 \"echo dut1\"");
    REQUEST("alias dut2 \"echo dut2\"");
    REQUEST("alias dut3 \"echo dut3\"");
    REQUEST("dut1;dut2;dut3");
    CHECK_RETCODE(0);
    EXPECT_STREQ(RESPONSE("dut1 \r\ndut2 \r\ndut3 "), buf);
}
TEST_F(mbedClientCli, cmd_alias_series)
{
    cmd_alias_add("multiecho", "echo dut1;echo dut2;");
    REQUEST("multiecho");
    CHECK_RETCODE(0);
    EXPECT_STREQ(RESPONSE("dut1 \r\ndut2 "), buf);
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_VARIABLES
TEST_F(mbedClientCli, cmd_var_1)
{
    REQUEST("set foo \"bar test\"");
    INIT_BUF();
    REQUEST("set");
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='/>'\r\n"
                 "?=0\r\n"
                 "foo='bar test'\r\n"
                 CMDLINE_EMPTY, buf);
    REQUEST("unset foo");
}

TEST_F(mbedClientCli, cmd_unset)
{
    REQUEST("set foo=a");
    REQUEST("unset foo");
    INIT_BUF();
    REQUEST("set");
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='/>'\r\n"
                 "?=0\r\n"
                 CMDLINE_EMPTY, buf);
}

TEST_F(mbedClientCli, cmd_var_2)
{
    REQUEST("set foo \"hello world\"");
    CHECK_RETCODE(0);
    REQUEST("echo foo");
    EXPECT_STREQ(RESPONSE("foo "), buf);

    REQUEST("echo $foo");
    CHECK_RETCODE(0);
    EXPECT_STREQ(RESPONSE("hello world "), buf);

    REQUEST("set faa !");
    REQUEST("echo $foo$faa");
    EXPECT_STREQ(RESPONSE("hello world! "), buf);
    REQUEST("unset faa");
}
#endif
#if MBED_CONF_CMDLINE_ENABLE_INTERNAL_COMMANDS
TEST_F(mbedClientCli, cmd__)
{
    REQUEST("echo foo");
    EXPECT_STREQ(RESPONSE("foo "), buf);
    REQUEST("_");
    EXPECT_STREQ(RESPONSE("foo "), buf);
}
TEST_F(mbedClientCli, var_prev_cmd)
{
    REQUEST("echo");
    REQUEST("set");
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='/>'\r\n"
                 "?=0\r\n"
                 CMDLINE_EMPTY, buf);
    REQUEST("invalid");
    REQUEST("set");
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='/>'\r\n"
                 "?=-5\r\n"
                 CMDLINE_EMPTY, buf);
}
TEST_F(mbedClientCli, var_ps1)
{
    REQUEST("set PS1=abc");
    EXPECT_STREQ(RAW_RESPONSE_WITH_PROMPT("", "abc"), buf);
    REQUEST("set")
    EXPECT_STREQ("\r\nvariables:\r\n"
                 "PS1='abc'\r\n"
                 "?=0\r\n"
                 "\r" ESCAPE("[2K") "abc " ESCAPE("[1D"), buf);
}

// operators
#if MBED_CONF_CMDLINE_ENABLE_OPERATORS
TEST_F(mbedClientCli, operator_semicolon)
{
    REQUEST("echo hello world")
    EXPECT_STREQ(RESPONSE("hello world "), buf);
    CHECK_RETCODE(CMDLINE_RETCODE_SUCCESS);

    REQUEST("setd faa \"hello world\";echo $faa");
    EXPECT_STREQ("\r\nCommand 'setd' not found.\r\n$faa \r\n" CMDLINE_EMPTY, buf);
}
TEST_F(mbedClientCli, operators_and)
{
    TEST_RETCODE_WITH_COMMAND("true && true", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("true && false", CMDLINE_RETCODE_FAIL);
    TEST_RETCODE_WITH_COMMAND("false && true", CMDLINE_RETCODE_FAIL);
    TEST_RETCODE_WITH_COMMAND("false && false", CMDLINE_RETCODE_FAIL);
}
TEST_F(mbedClientCli, operators_or)
{
    TEST_RETCODE_WITH_COMMAND("true || true", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("true || false", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("false || true", CMDLINE_RETCODE_SUCCESS);
    TEST_RETCODE_WITH_COMMAND("false || false", CMDLINE_RETCODE_FAIL);
}

TEST_F(mbedClientCli, ampersand)
{
    REQUEST("echo hello world&");
    EXPECT_STREQ(RESPONSE("hello world "), buf);
}
#endif
#endif
TEST_F(mbedClientCli, maxlength)
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
    //EXPECT_STREQ( RESPONSE((test_data+5)), buf);
}

#define REDIR_DATA "echo Hi!"
#define PASSTHROUGH_BUF_LENGTH 10
char passthrough_buffer[PASSTHROUGH_BUF_LENGTH];
char *passthrough_ptr = NULL;
void passthrough_cb(uint8_t c)
{
    if (passthrough_ptr != NULL) {
        *passthrough_ptr++ = c;
    }
}
TEST_F(mbedClientCli, passthrough_set)
{
    passthrough_ptr = passthrough_buffer;
    memset(&passthrough_buffer, 0, PASSTHROUGH_BUF_LENGTH);
    INIT_BUF();

    cmd_input_passthrough_func(passthrough_cb);
    input(REDIR_DATA);

    ASSERT_TRUE(strlen(buf) == 0);
    EXPECT_STREQ(REDIR_DATA, passthrough_buffer);

    cmd_input_passthrough_func(NULL);
    INIT_BUF();
    REQUEST(REDIR_DATA);
#if MBED_CONF_CMDLINE_INIT_AUTOMATION_MODE == 1
    EXPECT_STREQ("retcode: 0\r\n", buf);
#else
    EXPECT_STREQ(RESPONSE("Hi! "), buf);
#endif
}
TEST_F(mbedClientCli, passthrough_lf)
{
    passthrough_ptr = passthrough_buffer;
    memset(&passthrough_buffer, 0, PASSTHROUGH_BUF_LENGTH);
    input("\n");
    INIT_BUF();
    cmd_input_passthrough_func(passthrough_cb);
    input(REDIR_DATA);
    EXPECT_EQ(strlen(buf), 0);
    EXPECT_STREQ(REDIR_DATA, passthrough_buffer);
}
TEST_F(mbedClientCli, passthrough_cr)
{
    passthrough_ptr = passthrough_buffer;
    memset(&passthrough_buffer, 0, PASSTHROUGH_BUF_LENGTH);
    input("\r");
    INIT_BUF();
    cmd_input_passthrough_func(passthrough_cb);
    input(REDIR_DATA);
    ASSERT_TRUE(strlen(buf) == 0);
    EXPECT_STREQ(REDIR_DATA, passthrough_buffer);
}
TEST_F(mbedClientCli, passthrough_crlf)
{
    passthrough_ptr = passthrough_buffer;
    memset(&passthrough_buffer, 0, PASSTHROUGH_BUF_LENGTH);
    input("\r");
    INIT_BUF();
    cmd_input_passthrough_func(passthrough_cb);
    input("\n");
    input(REDIR_DATA);
    ASSERT_TRUE(strlen(buf) == 0);
    EXPECT_STREQ(REDIR_DATA, passthrough_buffer);
}
int cmd_long_called = 0;
int cmd_long(int argc, char *argv[])
{
    cmd_long_called ++;
    return CMDLINE_RETCODE_EXCUTING_CONTINUE;
}
TEST_F(mbedClientCli, cmd_continue)
{
    cmd_add("long", cmd_long, 0, 0);
    previous_retcode = 111;
    TEST_RETCODE_WITH_COMMAND("long", previous_retcode);
    cmd_ready(0);
    CHECK_RETCODE(CMDLINE_RETCODE_SUCCESS);
    EXPECT_EQ(cmd_long_called, 1);
}
TEST_F(mbedClientCli, cmd_out_func_set_null)
{
    cmd_out_func(NULL);
}

static int outf_called = 0;
void outf(const char *fmt, va_list ap)
{
    outf_called++;
}
TEST_F(mbedClientCli, cmd_out_func_set)
{
    outf_called = 0;
    cmd_out_func(&outf);
    // cppcheck-suppress formatExtraArgs
    cmd_vprintf(NULL, NULL);
    EXPECT_EQ(outf_called, 1);
}

TEST_F(mbedClientCli, cmd_ctrl_func_set_null)
{
    cmd_ctrl_func(NULL);
}

int sohf_cb_called = 0;
void sohf_cb(uint8_t c)
{
    sohf_cb_called++;
}
#if MBED_CONF_CMDLINE_ENABLE_ESCAPE_HANDLING == 1
TEST_F(mbedClientCli, cmd_ctrl_func_set)
{
    cmd_ctrl_func(sohf_cb);
    REQUEST("\x04");
    EXPECT_EQ(sohf_cb_called, 1);
    cmd_ctrl_func(NULL);
}
#endif

TEST_F(mbedClientCli, cmd_delete_null)
{
    cmd_delete(NULL);
}
#if MBED_CONF_CMDLINE_ENABLE_HISTORY
TEST_F(mbedClientCli, cmd_history_size_set)
{
    cmd_history_size(0);
    EXPECT_EQ(cmd_history_size(1), 1);
}
#endif
TEST_F(mbedClientCli, cmd_add_invalid_params)
{
    cmd_add(NULL, cmd_dummy, NULL, NULL);
    cmd_add("", cmd_dummy, NULL, NULL);
    cmd_add("abc", NULL, NULL, NULL);
}
/*
// @todo need more work to get in track
TEST_F(mbedClientCli, cmd_parameter_timestamp_1)
{
    int argc = 3;
    char *argv[] = {"cmd", "-t", "12345,6789"};
    const char *key = "-t";
    int64_t value = 0;
    // for some reason this causes crash when first strtok is called.!?!? Perhaps some bug?
    EXPECT_EQ(true, cmd_parameter_timestamp(argc, argv, key, &value));
    EXPECT_EQ(809048709, value);
}
*/
TEST_F(mbedClientCli, cmd_parameter_timestamp_2)
{
    int argc = 3;
    char *argv[] = {"cmd", "-t", "00:00:00:12:34:56:78:90"};
    const char *key = "-t";
    int64_t value = 0;
    EXPECT_EQ(true, cmd_parameter_timestamp(argc, argv, key, &value));
    EXPECT_EQ(78187493520, value);
}
TEST_F(mbedClientCli, cmd_parameter_timestamp_3)
{
    int argc = 3;
    char *argv[] = {"cmd", "-t", "12345"};
    const char *key = "-t";
    int64_t value = 0;
    EXPECT_EQ(true, cmd_parameter_timestamp(argc, argv, key, &value));
    EXPECT_EQ(12345, value);
}
TEST_F(mbedClientCli, cmd_parameter_timestamp_4)
{
    int argc = 3;
    char *argv[] = {"cmd", "-t", ":"};
    const char *key = "-t";
    int64_t value = 0;
    EXPECT_EQ(false, cmd_parameter_timestamp(argc, argv, key, &value));
    EXPECT_EQ(0, value);
}
TEST_F(mbedClientCli, cmd_parameter_timestamp_5)
{
    int argc = 3;
    char *argv[] = {"cmd", "-tt", "123"};
    const char *key = "-t";
    int64_t value = 0;
    EXPECT_EQ(false, cmd_parameter_timestamp(argc, argv, key, &value));
    EXPECT_EQ(0, value);
}
TEST_F(mbedClientCli, cmd_free)
{
    INIT_BUF();
    cmd_free();
    // these should not do anything since
    // library is not initialized anymore
    cmd_exe("");
    cmd_ready(0);
    cmd_next(0);
    EXPECT_STREQ("", buf);
}
