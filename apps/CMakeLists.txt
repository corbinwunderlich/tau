add_executable(tau cli/main.cpp)
target_link_libraries(tau PRIVATE libtau)
target_precompile_headers(tau REUSE_FROM libtau)
