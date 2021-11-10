# only build tests on targets that declare they are like posix
if(DEFINED LINUXIFY)

  project(mbed-client-cli-unittest)
  cmake_minimum_required(VERSION 3.11)

  include(FindUnixCommands)

  option(enable_coverage_data "Enable Coverage data" OFF)

  enable_testing ()

  if (enable_coverage_data)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif ()

  # Google Tests
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
  )

  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
   
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

    # GTest framework requires C++ version 11
    set (CMAKE_CXX_STANDARD 11)
    
    target_compile_definitions(mbed-client-cli-${TEST} PUBLIC ${flag})

    # describe the test executable
    add_executable(mbed_client_cli_test_${TEST} Test.cpp)

    # describe what the test executable needs to link with
    target_link_libraries(mbed_client_cli_test_${TEST}
      "mbed-client-cli-${TEST}"
      "mbed-trace"
      gtest_main
      )

    target_compile_definitions(mbed_client_cli_test_${TEST} PUBLIC ${flag})
    add_test(NAME mbed_client_cli_test_${TEST} COMMAND mbed_client_cli_test_${TEST})

    include(GoogleTest)
    gtest_discover_tests(mbed_client_cli_test_${TEST} EXTRA_ARGS --gtest_output=xml: XML_OUTPUT_DIR mbed_client_cli_test_${TEST})

    if (enable_coverage_data AND ${CMAKE_PROJECT_NAME} STREQUAL "mbed-client-cli-unittest")
      file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html")

      # get path mbed-client-cli top level as coverage is calculated from sources
      get_filename_component(CLI_TOP_LEVEL ../ ABSOLUTE)

      add_test(NAME mbed_client_cli_test_${TEST}_cov WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${BASH} -c "gcovr --root ${CLI_TOP_LEVEL} -e 'Test.cpp' -e '.*build.*' --html --html-details -o ${CMAKE_CURRENT_BINARY_DIR}/html/coverage_index.html"
      )
    endif ()
  endforeach()

endif()
