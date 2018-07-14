
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
#include "mbed-trace/mbed_trace.h"
#include "ns_cmdline.h"

#define TRACE_GROUP                 "main"

// dummy command with some option
static int cmd_dummy(int argc, char *argv[]) {
  if( cmd_has_option(argc, argv, "o") ) {
    cmd_printf("This is o option");
  } else {
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
  }
  return CMDLINE_RETCODE_SUCCESS;
}

int main(void)
{
    // Initialize trace library
    mbed_trace_init();
    cmd_init( 0 );              // initialize cmdline with print function
    cmd_add("dummy", cmd_dummy,
      "dummy command",
      "This is dummy command, which does not do anything except\n"
      "print text when o -option is given."); // add one dummy command

    tr_info("write 'help' and press ENTER");
    while(true) {
        int c = getchar();
        if (c != EOF) {
            cmd_char_input(c);
        }
    }
}
