
/*
 * Copyright (c) 2018 ARM Limited. All rights reserved.
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
#include <stdio.h>
#include <ncurses.h>
#include "mbed-trace/mbed_trace.h"
#include "ns_cmdline.h"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

// dummy command with some option
static int cmd_dummy(int argc, char *argv[]) {
  if( cmd_has_option(argc, argv, "o") ) {
    cmd_printf("This is o option\r\n");
  } else {
    return CMDLINE_RETCODE_INVALID_PARAMETERS;
  }
  return CMDLINE_RETCODE_SUCCESS;
}
volatile bool running = true;
static int cmd_exit(int argc, char *argv[]) {
  cmd_printf("Exiting cli.");
  running = false;
  return CMDLINE_RETCODE_SUCCESS;
}


int main(void)
{
    initscr();    // Start curses mode
    raw();        // Line buffering disabled
    noecho();     // Don't echo() while we do getch

    // Initialize trace library
    mbed_trace_init();
    cmd_init( 0 ); // initialize cmdline with print function
    cmd_add("exit", cmd_exit, "exit shell", 0);
    cmd_add("dummy", cmd_dummy,
      "dummy command",
      "This is dummy command, which does not do anything except\n"
      "print text when o -option is given."); // add one dummy command

    tr_info("write 'help' and press ENTER");
    cmd_output();
    while(running) {
        int c = getch();
        switch(c) {
          case(CTRL('c')):
            running = false;
            break;
          case(EOF):
            break;
          default:
            cmd_char_input(c);
        }
    }
    endwin();
    return 0;
}
