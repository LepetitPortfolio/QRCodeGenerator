message(STATUS "QRCodeGenerator - CMkeList")

find_package(Threads REQUIRED)

set(SRCROOT ${PROJECT_SOURCE_DIR}/src/QRCodeGenerator)

file(GLOB_RECURSE SRC_SOURCES
  "${SRCROOT}/*.cpp"
  "${SRCROOT}/*.cxx"
  "${SRCROOT}/*.cc"
)

# Also add headers to the IDE view (optional)
file(GLOB_RECURSE SLE_HEADERS
  "${SRCROOT}/*.h"
  "${SRCROOT}/*.hpp"
)

add_library(QRCodeGenerator STATIC ${SRC_SOURCES} ${SLE_HEADERS})
