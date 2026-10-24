if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

find_package(Catch2 3 REQUIRED)

add_executable(tests main.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain libtau)

include(Catch)

catch_discover_tests(tests)
