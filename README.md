# mbed-client-cli

This is the Command Line Library for a CLI application. It uses only ansi C features so it is portable and works in mbed-os-3, mbed-os-5, linux and windows.

## Features

Library provides features such:

* Adding commands to the interpreter.
* Deleting commands from the interpreter.
* Executing commands.
* Adding command aliases to the interpreter.
* Searching command arguments.
* implements several VT100/VT220 features, e.g.
  * move cursor left/right (or skipping word by pressing alt+up/right)
  * delete characters
  * CTRL+W to remove previous word
  * browse command history by pressing up/down
* implements basic commands, e.g.
  * echo
  * help
  * (un)set
  * alias
  * history
  * true/false
  * clear

## API

Command Line Library basic API's is described in the snipplet below:

```c++
// if thread safety for CLI terminal output is needed
// configure output mutex wait cb before initialization so it's available immediately
cmd_set_mutex_wait_func( (func)(void) );
// configure output mutex release cb before initialization so it's available immediately
cmd_set_mutex_wait_func( (func)(void) );
// initialize cmdline with print function
cmd_init( (func)(const char* fmt, va_list ap) );
// configure ready cb
cmd_set_ready_cb( (func)(int retcode)  );
// register command for library
cmd_add( <command>, (int func)(int argc, char *argv[]), <help>, <man>);
//execute some existing commands
cmd_exe( <command> );
```

Full API is described [here](mbed-client-cli/ns_cmdline.h)

### Confiration

Following defines can be used to configure defaults:

|define|type|default value|description|
|------|----|-------------|-----------|
|`MBED_CMDLINE_BOOT_MESSAGE`|C string|``|default boot message|
|`MBED_CMDLINE_MAX_LINE_LENGTH`|int|2000|maximum command line length|
|`MBED_CMDLINE_ARGUMENTS_MAX_COUNT`|int|30|maximum count of command arguments|
|`MBED_CMDLINE_HISTORY_MAX_COUNT`|int|32|maximum history size|
|`MBED_CMDLINE_INCLUDE_MAN`|bool|true|switching off ignore all man pages - to save used flash memory size|

### Pre defines return codes

each command should return some of pre-defines return codes.
These codes are reserved and used in test tools.

|define|description|
|------|-----------|
|`CMDLINE_RETCODE_COMMAND_BUSY`|Command Busy|
|`CMDLINE_RETCODE_EXCUTING_CONTINUE`|Execution continue in background|
|`CMDLINE_RETCODE_SUCCESS`|Execution Success|
|`CMDLINE_RETCODE_FAIL`|Execution Fail|
|`CMDLINE_RETCODE_INVALID_PARAMETERS`|Command parameters was incorrect|
|`CMDLINE_RETCODE_COMMAND_NOT_IMPLEMENTED`|Command not implemented|
|`CMDLINE_RETCODE_COMMAND_CB_MISSING`|Command callback function missing|
|`CMDLINE_RETCODE_COMMAND_NOT_FOUND`|Command not found|

## Tracing

Command Line Library has trace messages, which are disabled by default.
"MBED_CLIENT_CLI_TRACE_ENABLE" flag if defined, enables all the trace prints for debugging.

## Usage example

See full examples [here](example).

###
Adding new commands to the Command Line Library and executing the commands:

```c++
//example print function
void myprint(const char* fmt, va_list ap){ vprintf(fmt, ap); }

// ready cb, calls next command to be executed
void cmd_ready_cb(int retcode) { cmd_next( retcode ); }

// dummy command with some option
int cmd_dummy(int argc, char *argv[]){
  if( cmd_has_option(argc, argv, "o") ) {
    cmd_printf("This is o option");
  } else {
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
  }
  return CMDLINE_RETCODE_SUCCESS;
}

// timer cb (pseudo-timer-code)
void timer_ready_cb(void) {
   cmd_ready(CMDLINE_RETCODE_SUCCESS);
}

// long command, that needs for example some events to complete the command execution
int cmd_long(int argc, char *argv[] ) {
   timer_start( 5000, timer_ready_cb ); //pseudo timer code
   return CMDLINE_RETCODE_EXCUTING_CONTINUE;
}
void main(void) {
   cmd_init( &myprint );              // initialize cmdline with print function
   cmd_set_ready_cb( cmd_ready_cb );  // configure ready cb
   cmd_add("dummy", cmd_dummy, 0, 0); // add one dummy command
   cmd_add("long", cmd_long, 0, 0);   // add one dummy command
   //execute dummy and long commands
   cmd_exe( "dummy;long" );
}
```

## Thread safety
The CLI library is not thread safe, but the CLI terminal output can be locked against other
output streams, for example if both traces and CLI output are using serial out.

Thread safety example for mbed-os-5 is available [here](example/mbed-os-5/main.cpp).


## Unit tests

Unit tests is written using [yotta](https://yottabuild.org). To run unit tests:

* In Linux

```
yotta target x86-linux-native
yotta test
```

* In Mac

```
yotta target x86-osx-native
yotta test
```

* In Windows

```
yotta target x86-windows-native
yotta test
```
