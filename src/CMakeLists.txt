file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "*.cpp")

add_library(libtau STATIC ${SRC_FILES})
set_target_properties(libtau PROPERTIES OUTPUT_NAME tau)

target_include_directories(
  libtau
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
         $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
