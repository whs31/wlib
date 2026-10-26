find_package(Catch2 REQUIRED)
find_package(glm REQUIRED)

set(WLIB_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB WLIB_TEST_SOURCES CONFIGURE_DEPENDS "${WLIB_TESTS_DIR}/*.cc")
file(GLOB WLIB_GLAZE_TEST_SOURCES CONFIGURE_DEPENDS "${WLIB_TESTS_DIR}/*_glaze.cc")
list(REMOVE_ITEM WLIB_TEST_SOURCES
  "${WLIB_TESTS_DIR}/test_main.cc"
  ${WLIB_GLAZE_TEST_SOURCES}
)

add_executable(${PROJECT_NAME}_tests
  "${WLIB_TESTS_DIR}/test_main.cc"
  ${WLIB_TEST_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_tests
  PRIVATE
  w::lib
  Catch2::Catch2
  glm::glm
)

# note: glm is a test-only oracle for the linear algebra; gtx comparisons need the experimental
# extensions
target_compile_definitions(${PROJECT_NAME}_tests PRIVATE GLM_ENABLE_EXPERIMENTAL)

if(WLIB_WITH_GLAZE)
  target_sources(${PROJECT_NAME}_tests PRIVATE ${WLIB_GLAZE_TEST_SOURCES})
  target_link_libraries(${PROJECT_NAME}_tests PRIVATE w::lib::glaze)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ${PROJECT_NAME}_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}_tests>
      $<TARGET_FILE_DIR:${PROJECT_NAME}_tests>
    COMMAND_EXPAND_LISTS
  )
endif ()

add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
