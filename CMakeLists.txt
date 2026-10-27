cmake_minimum_required(VERSION 3.20)

message(DEBUG "Using ${CMAKE_TOOLCHAIN_FILE}")
message(DEBUG "Tests: ${BUILD_TESTING}")

project(wlib
  VERSION 8.0.0
  DESCRIPTION "Common C++23 utility library"
  HOMEPAGE_URL "https://git.whs31.com/shared/support/wlib"
  LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(WLIB_WITH_GLAZE "Build the glaze adapters (w::lib::glaze)" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
if(WLIB_WITH_GLAZE)
  find_package(glaze REQUIRED)
endif()

add_subdirectory(config)

add_library(${PROJECT_NAME})
add_library(w::lib ALIAS ${PROJECT_NAME})
set(WLIB_INSTALL_TARGETS ${PROJECT_NAME})

if(WLIB_WITH_GLAZE)
  add_library(${PROJECT_NAME}_glaze INTERFACE)
  add_library(w::lib::glaze ALIAS ${PROJECT_NAME}_glaze)
  set_target_properties(${PROJECT_NAME}_glaze PROPERTIES EXPORT_NAME lib::glaze)
  list(APPEND WLIB_INSTALL_TARGETS ${PROJECT_NAME}_glaze)
endif()

# The public headers use C++23, so the standard is a usage requirement: a consumer that includes
# them compiles as C++23 too, whatever its own default.
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_EXTENSIONS OFF
  DEFINE_SYMBOL W_COMMON_LIBRARY
  EXPORT_NAME lib
  LINKER_LANGUAGE CXX
  POSITION_INDEPENDENT_CODE ON
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
  BASE_NAME W_COMMON
  EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/w/common/detail/export.h"
  EXPORT_MACRO_NAME W_COMMON_API
  STATIC_DEFINE W_COMMON_STATIC_LIBRARY
)

file(GLOB_RECURSE WLIB_PUBLIC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/w/common/*.h")
# The serialization adapters belong to their optional target, not to the core library.
list(FILTER WLIB_PUBLIC_HEADERS EXCLUDE REGEX "/include/w/common/glaze/")
file(GLOB WLIB_PRIVATE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/private/*.h"
)

target_sources(${PROJECT_NAME} PRIVATE ${WLIB_PUBLIC_HEADERS} ${WLIB_PRIVATE_SOURCES})

# Deterministic random streams must round every operation exactly as IEEE 754 prescribes, so
# multiply-adds may not be contracted into FMA instructions (Clang contracts by default, and so
# does GCC in GNU mode or on targets with FMA).
set_source_files_properties(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/random.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/noise.cc"
  PROPERTIES COMPILE_OPTIONS
    "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-ffp-contract=off>;$<$<CXX_COMPILER_ID:MSVC>:/fp:precise>"
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  w::lib::config

  fmt::fmt
  spdlog::spdlog
)

if(WLIB_WITH_GLAZE)
  target_link_libraries(${PROJECT_NAME}_glaze INTERFACE
    w::lib
    glaze::glaze
  )
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

if (NOT BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC W_COMMON_STATIC_LIBRARY)
endif ()

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

install(TARGETS ${WLIB_INSTALL_TARGETS}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/w/common"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w"
  FILES_MATCHING PATTERN "*.h"
  PATTERN "glaze" EXCLUDE
)
if(WLIB_WITH_GLAZE)
  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/w/common/glaze"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w/common"
    FILES_MATCHING PATTERN "*.h"
  )
endif()
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/w/common/detail/export.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w/common/detail"
)
install(EXPORT ${PROJECT_NAME}Targets
  FILE WLibTargets.cmake
  NAMESPACE w::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/WLib"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/WLibConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/WLibConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/WLib"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/WLibConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/WLibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/WLibConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/WLib"
)
