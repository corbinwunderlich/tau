find_package(Catch2 3 REQUIRED)

add_executable(tau_tests main.cpp)

target_link_libraries(tau_tests PRIVATE Catch2::Catch2WithMain tau_core)
