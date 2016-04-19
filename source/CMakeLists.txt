if(DEFINED TARGET_LIKE_X86_LINUX_NATIVE)
    include_directories("yotta_modules/nanostack-libservice/source/")
    add_library( mbed-client-cli
        ns_cmdline.c
        ../yotta_modules/nanostack-libservice/source/libList/ns_list.c
    )
    add_definitions("-g -O0 -fprofile-arcs -ftest-coverage")
    target_link_libraries(mbed-client-cli gcov)
else()
    include_directories("yotta_modules/nanostack-libservice/source/")
    add_library( mbed-client-cli
        ns_cmdline.c
        ../yotta_modules/nanostack-libservice/source/libList/ns_list.c
    )
    target_link_libraries(mbed-client-cli)
endif()
