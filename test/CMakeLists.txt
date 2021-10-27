# only build tests on targets that declare they are like posix
if(DEFINED LINUXIFY)

  project(mbed-client-cli-unittest)
  cmake_minimum_required(VERSION 3.16)

  # CppUTest
  include(FetchContent)
  FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        latest-passing-build # or use release tag, eg. v3.8
    )
  # Set this to ON if you want to have the CppUTests in your project as well.
  set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")
  FetchContent_MakeAvailable(CppUTest)

  add_definitions("-Wno-write-strings")

  add_library(mbed-trace INTERFACE)
  target_include_directories(mbed-trace INTERFACE
    ../example/linux
    )

  # FULL BUILD TESTS
  set(FLAGS
    MBED_CONF_CMDLINE_USE_MINIMUM_SET=0
    MBED_CONF_CMDLINE_USE_MINIMUM_SET=1
    )
  set(TESTS
    full
    min
    )
  foreach(flag ${FLAGS})
    list(FIND FLAGS ${flag} index)
    list(GET TESTS ${index} TEST)

    MESSAGE("TEST flag: ${flag}")

    add_library( mbed-client-cli-${TEST}
      ../source/ns_cmdline.c
      ../source/ns_list_internal/ns_list.c
      )
    target_link_libraries(mbed-client-cli-${TEST} mbed-trace)

    target_include_directories(mbed-client-cli-${TEST}
      PRIVATE
      ../source/ns_list_internal
      )

    target_include_directories(mbed-client-cli-${TEST}
      PUBLIC
      ..
      ../mbed-client-cli
      )

    if("${TEST}" STREQUAL "full")
      MESSAGE("USE GCOV BUILD (Linux)")
      set_target_properties(mbed-client-cli-${TEST} PROPERTIES COMPILE_FLAGS
        "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
        )
      target_link_libraries(mbed-client-cli-${TEST} gcov)
    else()
      target_link_libraries(mbed-client-cli-${TEST})
    endif()
    target_compile_definitions(mbed-client-cli-${TEST} PUBLIC ${flag})

    # describe the test executable
    add_executable(mbed_client_cli_test_${TEST} Test.cpp)

    # describe what the test executable needs to link with
    target_link_libraries(mbed_client_cli_test_${TEST}
      "mbed-client-cli-${TEST}"
      "mbed-trace"
      CppUTest
      )

    target_compile_definitions(mbed_client_cli_test_${TEST} PUBLIC ${flag})

    # describe what is actual test binary
    SET(TEST_EXECUTABLE "../../../build/x86-linux-native/test/mbed_client_cli_test_${TEST}")
    add_test(mbed_client_cli_test_${TEST} ${TEST_EXECUTABLE})
  endforeach()

else()

if(DEFINED TARGET_LIKE_X86_WINDOWS_NATIVE OR DEFINED TARGET_LIKE_X86_LINUX_NATIVE OR DEFINED TARGET_LIKE_X86_OSX_NATIVE)

    include_directories("../yotta_modules/cpputest")
    add_definitions("-Wno-write-strings")

    # FULL BUILD TESTS
    set(FLAGS
            MBED_CONF_CMDLINE_USE_MINIMUM_SET=0
            MBED_CONF_CMDLINE_USE_MINIMUM_SET=1
            )
    set(TESTS
            full
            min
            )
    foreach(flag ${FLAGS})
        list(FIND FLAGS ${flag} index)
        list(GET TESTS ${index} TEST)

        MESSAGE("TEST flag: ${flag}")

        add_library( mbed-client-cli-${TEST}
            ../source/ns_cmdline.c
            ../source/ns_list_internal/ns_list.c
            )

        if("${TEST}" STREQUAL "full")
            if(DEFINED TARGET_LIKE_X86_LINUX_NATIVE)
                MESSAGE("USE GCOV BUILD (Linux)")
                set_target_properties(mbed-client-cli-${TEST} PROPERTIES COMPILE_FLAGS
                        "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
                        )
                target_link_libraries(mbed-client-cli-${TEST} gcov)
            #[[
            elseif(DEFINED TARGET_LIKE_X86_OSX_NATIVE)
                # Not supported yet..
                MESSAGE("USE GCOV BUILD (Mac)")
                #target_link_libraries(mbed-client-cli-${TEST} clang_rt)
            #]]
            else()
                MESSAGE("gcov is not supported in Windows")
                target_link_libraries(mbed-client-cli-${TEST})
            endif()
        else()
            target_link_libraries(mbed-client-cli-${TEST})
        endif()
        target_compile_definitions(mbed-client-cli-${TEST} PUBLIC ${flag})

        # describe the test executable
        add_executable(mbed_client_cli_test_${TEST} Test.cpp)

        # describe what the test executable needs to link with
        target_link_libraries(mbed_client_cli_test_${TEST}
                "mbed-client-cli-${TEST}"
                "mbed-trace"
                cpputest
                )

        target_compile_definitions(mbed_client_cli_test_${TEST} PUBLIC ${flag})

        # describe what is actual test binary
        if(DEFINED TARGET_LIKE_X86_WINDOWS_NATIVE)
            SET(TEST_EXECUTABLE "build/x86-windows-native/test/mbed_client_cli_test_${TEST}")
        elseif(DEFINED TARGET_LIKE_X86_OSX_NATIVE)
            SET(TEST_EXECUTABLE "../../../build/x86-osx-native/test/mbed_client_cli_test_${TEST}")
        elseif(DEFINED TARGET_LIKE_X86_LINUX_NATIVE)
            SET(TEST_EXECUTABLE "../../../build/x86-linux-native/test/mbed_client_cli_test_${TEST}")
        endif()
        add_test(mbed_client_cli_test_${TEST} ${TEST_EXECUTABLE})
    endforeach()

endif()

endif()
