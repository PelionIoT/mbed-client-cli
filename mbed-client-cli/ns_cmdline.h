/*
 * Copyright (c) 2014-2015 ARM. All rights reserved.
 */
/**
 * \file ns_cmdline.h
 * 
 * Command line library - mbedOS shell
 *
 * Usage example:
 *
 * \code
 * //simple print function
 * void myprint(const char* fmt, va_list ap){ vprintf(fmt, ap); }
 * // simple ready cb, which call next command to be execute
 * void cmd_ready_cb(int retcode) { cmd_next( retcode ); }
 *
 * // dummy command with some option
 * int cmd_dummy(int argc, char *argv[]){
 *  if( cmd_has_option(argc, argv, "o") ) {
 *    cmd_printf("This is o option");
 *  } else {
 *    return CMDLINE_RETCODE_INVALID_PARAMETERS;
 *  }
 *  return CMDLINE_RETCODE_SUCCESS;
 *}
 * // timer cb ( pseudo-timer-code )
 * void timer_ready_cb(void) {
 *   cmd_ready(CMDLINE_RETCODE_SUCCESS);
 * }
 * // long command, which need e.g. some events to finalize command execution
 * int cmd_long(int argc, char *argv[] ) {
     timer_start( 5000, timer_ready_cb );
 *   return CMDLINE_RETCODE_EXCUTING_CONTINUE;
 * }
 * void main(void) {
 *   cmd_init( &myprint );              // initialize cmdline with print function
 *   cmd_set_ready_cb( cmd_ready_cb );  // configure ready cb
 *   cmd_add("dummy", cmd_dummy, 0, 0); // add one dummy command
 *   cmd_add("long", cmd_long, 0, 0);   // add one dummy command
 *   //execute dummy and long commands
 *   cmd_exe( "dymmy;long" );
 * }
 * \endcode
 * \startuml{cli_usecase.png}
   actor user
   participant app
   participant cli
   participant mycmd

 == Initialization ==
 * app -> cli: cmd_init( &default_cmd_response_out )
 note right : This initialize command line library
 * mycmd -> cli: cmd_add( mycmd_command, "mycmd", NULL, NULL)
 note right : All commands have to be register \nto cmdline library with cmd_add() function

 == command input characters==
 * app -> cli: cmd_char_input("d")
 * app -> cli: cmd_char_input("u")
 * app -> cli: cmd_char_input("m")
 * app -> cli: cmd_char_input("m")
 * app -> cli: cmd_char_input("y")
 * app -> cli: cmd_char_input("\\n")
 note left : User write command to \nbe execute and press ENTER when \ncommand with all parameters are ready.\nCharacters can be come from serial port for example.
 == command execution==
 * mycmd <- cli: mycmd_command(argc, argv)
 * mycmd -> cli: cmd_printf("hello world!\\n")
   note right : cmd_printf() should \nbe used when command prints \nsomething to the console
 * cli -> user: "hello world!\\n"
 * mycmd -> cli: <<retcode>>
 == finish command and goto forward ==
 * app <- cli: cmd_ready_cb()
 * app -> cli: cmd_next()
 note left : this way application can \ndecide when/how to go forward.\nWhen using event-loop, \nyou probably want create tasklet where \ncommands are actually executed.\nif there are some commands in queue cmd_next()\nstart directly next command execution.\n
 == command execution==
 * app -> cli: cmd_exe("long\\n")
 note left : input string can be \ngive also with cmd_exe() -function
 * mycmd <- cli: long_command(argc, argv)
 * mycmd -> cli: <<retcode>> = CMDLINE_RETCODE_EXECUTING_CONTINUE
 note right : When command continue in background, it should return\nCMDLINE_RETCODE_EXECUTING_CONTINUE.\nCommand interpreter not continue next command \nas long as cmd_ready() -function is not called.
 ... Some ~~long delay~~ ...
 * mycmd -> cli: cmd_ready( <<retcode>> )
 note right : When command is finally finished,\nit should call cmd_ready() function.
 == finish command and goto forward ==
 * app <- cli: cmd_ready_cb()
 * app -> cli: cmd_next()
 ... ...
 * \enduml
 */
#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#if defined(_WIN32) || defined(__unix__) || defined(__unix) || defined(unix) || defined(YOTTA_CFG)

#if defined(YOTTA_CFG)
#include "mbed-client-cli/ns_types.h"
#else
#include <stdint.h>
#include <stddef.h>
#endif

#else
#include "ns_types.h"
#endif

#define CMDLINE_RETCODE_COMMAND_BUSY            2   //!< Command Busy
#define CMDLINE_RETCODE_EXCUTING_CONTINUE       1   //!< Execution continue in background
#define CMDLINE_RETCODE_SUCCESS                 0   //!< Execution Success
#define CMDLINE_RETCODE_FAIL                    -1  //!< Execution Fail
#define CMDLINE_RETCODE_INVALID_PARAMETERS      -2  //!< Command parameters was incorrect
#define CMDLINE_RETCODE_COMMAND_NOT_IMPLEMENTED -3  //!< Command not implemented
#define CMDLINE_RETCODE_COMMAND_CB_MISSING      -4  //!< Command callback function missing
#define CMDLINE_RETCODE_COMMAND_NOT_FOUND       -5  //!< Command not found

/**
 * typedef for print functions
 */
typedef void (cmd_print_t)(const char *, va_list);
/**
 * Initialize cmdline class.
 * This is command line editor without any commands. Application
 * needs to add commands that should be enabled.
 * usage e.g.
 * \code
    cmd_init( &default_cmd_response_out );
 * \endcode
 * \param outf  console printing function (like vprintf)
 */
void cmd_init(cmd_print_t *outf);
/** Command ready function for __special__ cases.
 * This need to be call if command implementation return CMDLINE_RETCODE_EXECUTING_CONTINUE
 * because there is some background stuff ongoing before command is finally completed.
 * Normally there is some event, which call cmd_ready().
 * \param retcode return code for command
 */
void cmd_ready(int retcode);
/** typedef for ready cb function */
typedef void (cmd_ready_cb_f)(int);
/**
 * Configure cb which will be called after commands are executed
 * or cmd_ready is called
 * \param cb    callback function for command ready
 */
void cmd_set_ready_cb(cmd_ready_cb_f *cb);
/**
 * execute next command if any
 * \param retcode last command return value
 */
void cmd_next(int retcode);
/** Free cmd class */
void cmd_free(void);
/** Reset cmdline to default values
 *  detach external commands, delete all variables and aliases
 */
void cmd_reset(void);
/** Configure command history size (default 32)
 *  \param max  maximum history size
 *  max > 0 -> configure new value
 *  max = 0 -> just return current value
 *  \return current history max-size
 */
uint8_t cmd_history_size(uint8_t max);
/** command line print function
 *  This function should be used when user want to print something to the console
 *  \param fmt   console print function (like printf)
 */
void cmd_printf(const char *fmt, ...);
/** Reconfigure default cmdline out function (cmd_printf)
 *  \param outf  select console print function
 */
void cmd_out_func(cmd_print_t *outf);
/** Configure function, which will be called when Ctrl+A is pressed
 * \param sohf control function which called every time when user input control keys
 */
void cmd_ctrl_func(void (*sohf)(uint8_t c));
/** Refresh output */
void cmd_output(void);
/** default cmd response function, use stdout
 *  \param fmt  The format string is a character string, beginning and ending in its initial shift state, if any. The format string is composed of zero or more directives.
 *  \param ap   list of parameters needed by format string. This must correspond properly with the conversion specifier.
 */
void default_cmd_response_out(const char *fmt, va_list ap);
/** Initialize screen */
void cmd_init_screen(void);
/** Get echo state
 * \return true if echo is on otherwise false
 */
bool cmd_echo_state(void);
/** Echo off */
void cmd_echo_off(void);
/** Echo on */
void cmd_echo_on(void);
/** Enter character to console.
 * insert key pressess to cmdline called from main loop of application
 * \param u_data char to be added to console
 */
void cmd_char_input(int16_t u_data);


/* Methods used for adding and handling of commands and aliases
 */

/** Callback called when your command is run.
 * \param argc argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv argv is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 */
typedef int (cmd_run_cb)(int argc, char *argv[]);
/** Add command to intepreter
 * \param name      command string
 * \param callback  This function is called when command line start executing
 * \param info      Command short description which is visible in help command, or null if not in use
 * \param man       Help page for this command. This is shown when executing command with invalid parameters or command with --help parameter. Can be null if not in use.
 */
void cmd_add(const char *name, cmd_run_cb *callback, const char *info, const char *man);

/** delete command from intepreter
 *  \param name command to be delete
 */
void cmd_delete(const char *name);
/** Command executer.
 * Command executer, which split&push command(s) to the buffer and
 * start executing commands in cmd tasklet.
 * if not, execute command directly.
 * If command implementation returns CMDLINE_RETCODE_EXCUTING_CONTINUE,
 * executor will wait for cmd_ready() before continue to next command.
 * \param str  command string, e.g. "help"
 */
void cmd_exe(char *str);
/** Add alias to interpreter.
 * Aliases are replaced with values before executing a command. All aliases must be started from beginning of line.
 * null or empty value deletes alias.
 * \code
   cmd_alias_add("print", "echo");
   cmd_exe("print \"hello world!\""); // this is now same as "echo \"hello world!\"" .
 * \endcode
 * \param alias     alias name
 * \param value     value for alias. Values can be any visible ASCII -characters.
 */
void cmd_alias_add(char *alias, char *value);
/** Add Variable to interpreter.
 * Variables are replaced with values before executing a command.
 * To use variables from cli, use dollar ($) -character so that interpreter knows user want to use variable in that place.
 * null or empty value deletes variable.
 * \code
   cmd_variable_add("world", "hello world!");
   cmd_exe("echo $world"); // this is now same as echo "hello world!" .
 * \endcode
 * \param variable  Variable name, which will be replaced in interpreter.
 * \param value     Value for variable. Values can contains white spaces and '"' or '"' characters.
 */
void cmd_variable_add(char *variable, char *value);

/** find command parameter index by key.
 * e.g.
 * \code
      int main(void){
            //..init cmd..
            //..
            cmd_exe("mycmd enable")
      }
      int mycmd_command(int argc, char *argv[]) {
        bool found = cmd_parameter_index( argc, argv, "enable" ) > 0;
      }
 * \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \param key   option key, which index you want to find out.
 * \return index where parameter was or -1 when not found
 */
int cmd_parameter_index(int argc, char *argv[], char *key);
/** check if command option is present.
 * e.g. cmd: "mycmd -c"
 * \code
 *    bool on = cmd_has_option( argc, argv, "p" );
 * \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \param key   option key to be find
 * \return true if option found otherwise false
 */
bool cmd_has_option(int argc, char *argv[], char *key);
/** find command parameter by key.
 * if exists, return true, otherwise false.
 * e.g. cmd: "mycmd enable 1"
 * \code
  int mycmd_command(int argc, char *argv[]) {
        bool value;
        bool found = cmd_parameter_bool( argc, argv, "mykey", &value );
        if( found ) return CMDLINE_RETCODE_SUCCESS;
        else return CMDLINE_RETCODE_FAIL;
    }
 * \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \param key   parameter key to be find
 * \param value parameter value to be fetch, if key not found value are untouched. "1" and "on" and "true" and "enable" and "allow" are True -value, all others false.
 * \return true if parameter key and value found otherwise false
 */
bool cmd_parameter_bool(int argc, char *argv[], char *key, bool *value);
/** find command parameter by key and return value (next parameter).
 * if exists, return parameter pointer, otherwise null.
 * e.g. cmd: "mycmd mykey myvalue"
 * \code
    int mycmd_command(int argc, char *argv[]) {
        char *value;
        bool found = cmd_parameter_val( argc, argv, "mykey", &value );
        if( found ) return CMDLINE_RETCODE_SUCCESS;
        else return CMDLINE_RETCODE_FAIL;
    }
 * \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \param key   parameter key to be find
 * \param value pointer to pointer, which will point to cli input data when key and value found. if key or value not found this parameter are untouched.
 * \return true if parameter key and value found otherwise false
 */
bool cmd_parameter_val(int argc, char *argv[], char *key, char **value);
/** find command parameter by key and return value (next parameter) in integer.
 * e.g. cmd: "mycmd mykey myvalue"
 * \code
 *   int32_t i;
 *   cmd_parameter_val_int( argc, argv, "mykey", &i );
 * \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \param key   parameter key to be find
 * \param value parameter value to be fetch, if key not found value are untouched.
 * \return true if parameter key and value found otherwise false
 */
bool cmd_parameter_int(int argc, char *argv[], char *key, int32_t *value);
/** Get last command line parameter as string.
 * e.g.
 * \code
    cmd: "mycmd hello world"
        cmd_parameter_last -> "world"
    cmd: "mycmd"
        cmd_parameter_last() -> NULL
   \endcode
 * \param argc  argc is the count of arguments given in argv pointer list. value begins from 1 and this means that the 0 item in list argv is a string to name of command.
 * \param argv  is list of arguments. List size is given in argc parameter. Value in argv[0] is string to name of command.
 * \return pointer to last parameter or NULL when there is no any parameters.
 */
char *cmd_parameter_last(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
#endif /*_CMDLINE_H_*/
