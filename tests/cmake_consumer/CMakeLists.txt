cmake_minimum_required(VERSION 3.20)

project(wlib_consumer LANGUAGES CXX)

set(WLIB_CONSUMER_COMPONENT "core" CACHE STRING "WLib component to test: core or glaze")
set_property(CACHE WLIB_CONSUMER_COMPONENT PROPERTY STRINGS core glaze)

if(WLIB_CONSUMER_COMPONENT STREQUAL "glaze")
  find_package(WLib REQUIRED COMPONENTS glaze)
  set(target w::lib::glaze)
elseif(WLIB_CONSUMER_COMPONENT STREQUAL "core")
  find_package(WLib REQUIRED)
  set(target w::lib)
else()
  message(FATAL_ERROR "Unknown WLIB_CONSUMER_COMPONENT: ${WLIB_CONSUMER_COMPONENT}")
endif()

# note: the consumer sets no language standard; w::lib requires C++23 of it
add_executable(wlib_consumer "${CMAKE_CURRENT_SOURCE_DIR}/${WLIB_CONSUMER_COMPONENT}.cc")
target_link_libraries(wlib_consumer PRIVATE ${target})
