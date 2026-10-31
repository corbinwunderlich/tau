if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "*.cpp")

add_executable(tests ${TEST_FILES})
target_link_libraries(
    tests
    PRIVATE
        Catch2::Catch2WithMain
        libtau
)
target_precompile_headers(tests REUSE_FROM libtau)

include(Catch)

catch_discover_tests(tests EXTRA_ARGS --colour-mode ANSI)
