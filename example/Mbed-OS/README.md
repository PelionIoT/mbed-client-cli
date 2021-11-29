# Mbed OS example application using mbed-client-cli library

## build

```
mbed deploy
mbed compile -t GCC_ARM -m K64F
```


## Usage

When you flash a target with this application and open a terminal you should see the following traces:

```
[INFO][main] write 'help' and press ENTER
>
...
```


## build steps for workaround for issue #81 (PR #82)

For the reason descript in [issue #81](https://github.com/ARMmbed/mbed-client-cli/issues/81), example folder moved out
of repository.
For cases like this, build steps are:
1. go to new example folder
2. check the revision and create `mbed-os.lib` and `mbed-client-cli.lib` lib files.
3. `mbed new .` or `mbed config root .` to make current directory import libs, then
```
mbed deploy
mbed compile -t GCC_ARM -m K64F
```
