# only build tests on targets that declare they are like posix
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
        if(DEFINED TARGET_LIKE_X86_LINUX_NATIVE)
            if(${TEST} STREQUAL "full")
                MESSAGE("USE GCOV BUILD")
                set_target_properties(mbed-client-cli-${TEST} PROPERTIES -fprofile-arcs -ftest-coverage)
                target_link_libraries(mbed-client-cli-${TEST} gcov)
            else()
                MESSAGE("use default test..")
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
