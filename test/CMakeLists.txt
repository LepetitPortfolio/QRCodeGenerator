message(STATUS "Test - CMkeList")

set(TESTSRCROOT ${PROJECT_SOURCE_DIR}/test)

file(GLOB_RECURSE GAME_SOURCES
  "${TESTSRCROOT}/*.cpp"
  "${TESTSRCROOT}/*.cxx"
  "${TESTSRCROOT}/*.cc"
)

# Also add headers to the IDE view (optional)
file(GLOB_RECURSE GAME_HEADERS
  "${TESTSRCROOT}/*.h"
  "${TESTSRCROOT}/*.hpp"
)

add_executable(Test ${GAME_SOURCES})

target_include_directories(Test PRIVATE ${CMAKE_SOURCE_DIR}/src/QRCodeGenerator)

