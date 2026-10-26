project(wlib_config LANGUAGES NONE)

add_library(${PROJECT_NAME} INTERFACE)
add_library(w::lib::config ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME lib::config)

# config.h.in tests the feature macros by their prefixed name, so they are set under that name.
foreach(feature IN ITEMS QT GLAZE)
  set(W_COMMON_WITH_${feature} ${WLIB_WITH_${feature}})
endforeach()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/w/common/config.h"
  @ONLY
)

target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/w/common/config.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w/common"
)

install(TARGETS ${PROJECT_NAME}
  EXPORT wlibTargets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
