# mbed-client-cli

Command Line Library for CLI application. Library provides methods for:

* adding commands to interpreter
* deleting comamnds from interpreter
* executing commands
* adding command aliases to interpreter
* searching command arguments
* etc...


## API

Command line library API is described in the snipplet below. 

```c++
// initialize cmdline with print function
cmd_init( (func)(const char* fmt, va_list ap) );
// configure ready cb.
cmd_set_ready_cb( (func)(int retcode)  );
// register command for library
cmd_add( <command>, (int func)(int argc, char *argv[]), <help>, <man>); 
//execute some existing commands
cmd_exe( <command> );
```

## Usage example

Here is an example how to add new commands to command line library and execute the commands.

```c++
//example print function
void myprint(const char* fmt, va_list ap){ vprintf(fmt, ap); }

// ready cb, which call next command to be execute
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

// timer cb ( pseudo-timer-code )
void timer_ready_cb(void) {
   cmd_ready(CMDLINE_RETCODE_SUCCESS);
}

// long command, which need e.g. some events to finalize command execution
int cmd_long(int argc, char *argv[] ) {
   timer_start( 5000, timer_ready_cb ); //pseudo timer code
   return CMDLINE_RETCODE_EXCUTING_CONTINUE;
}
void main(void) {
   cmd_init( &myprint );              // initialize cmdline with print function
   cmd_set_ready_cb( cmd_ready_cb );  // configure ready cb.
   cmd_add("dummy", cmd_dummy, 0, 0); // add one dummy command
   cmd_add("long", cmd_long, 0, 0);   // add one dummy command
   //execute dummy and long commands
   cmd_exe( "dymmy;long" );
}
```
