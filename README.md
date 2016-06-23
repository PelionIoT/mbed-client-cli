# mbed-client-cli

This is the Command Line Library for a CLI application. This library provides methods for:

* Adding commands to the interpreter.
* Deleting commands from the interpreter.
* Executing commands.
* Adding command aliases to the interpreter.
* Searching command arguments.

## API

Command Line Library API is described in the snipplet below: 

```c++
// initialize cmdline with print function
cmd_init( (func)(const char* fmt, va_list ap) );
// configure ready cb
cmd_set_ready_cb( (func)(int retcode)  );
// register command for library
cmd_add( <command>, (int func)(int argc, char *argv[]), <help>, <man>); 
//execute some existing commands
cmd_exe( <command> );
// if thread safety for CLI terminal output is needed
// configure output mutex wait cb
cmd_set_mutex_wait_func( (func)(void) );
// configure output mutex release cb 
cmd_set_mutex_wait_func( (func)(void) );
```

## Usage example

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

```c++
static Mutex MyMutex;

// mutex wait cb, acquires the mutex, waiting if necessary
static void mutex_wait(void)
{
    MyMutex.lock();
}

// mutex release cb, releases the mutex
static void my_mutex_release(void)
{
    MyMutex.unlock();
}

void main(void) {
   cmd_init( &myprint );              // initialize cmdline with print function
   cmd_set_ready_cb( cmd_ready_cb );  // configure ready cb.
   cmd_mutex_wait_func( my_mutex_wait ); // configure mutex wait function
   cmd_mutex_release_func( my_mutex_release ); // configure mutex release function
   //CLI terminal output now locks against MyMutex
}

```
