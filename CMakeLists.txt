cmake_minimum_required (VERSION 3.24)

project(rgb-tui
  LANGUAGES CXX
  VERSION 1.2.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS FALSE)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY "https://github.com/ArthurSonzogni/ftxui"
  GIT_TAG v6.0.0
  GIT_PROGRESS TRUE
  GIT_SHALLOW
  EXCLUDE_FROM_ALL
)

FetchContent_Declare(fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "2cac8a9"
  GIT_PROGRESS TRUE
  GIT_SHALLOW
  EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(clip
  GIT_REPOSITORY "https://github.com/dacap/clip"
  GIT_TAG "1d5d20cefbe388b3cde285a4ec61114238d3fa70"
  GIT_PROGRESS TRUE
  GIT_SHALLOW 
  EXCLUDE_FROM_ALL
)
FetchContent_GetProperties(clip)
if(NOT clip_POPULATED)
  FetchContent_Populate(clip)
  add_subdirectory(${clip_SOURCE_DIR} ${clip_BINARY_DIR} EXCLUDE_FROM_ALL)
  target_include_directories(clip INTERFACE ${clip_SOURCE_DIR})
endif()

# ------------------------------------------------------------------------------

add_executable(rgb-tui src/main.cpp)
target_include_directories(rgb-tui PRIVATE src)

target_link_libraries(rgb-tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component # Not needed for this example.
  PRIVATE fmt::fmt
  PRIVATE clip
)

# C++17 is used. We requires fold expressions at least.
set_target_properties(rgb-tui PROPERTIES CXX_STANDARD 17)

# ------------------------------------------------------------------------------

install(TARGETS rgb-tui RUNTIME DESTINATION "bin")

# ------------------------------------------------------------------------------
if (UNIX AND NOT APPLE)
  set(CPACK_GENERATOR "DEB;External;RPM;STGZ;TBZ2;TGZ;TXZ;TZ;TZST;ZIP")
elseif (UNIX AND APPLE)
  set(CPACK_GENERATOR "DragNDrop;NuGet;TGZ;ZIP")
elseif (WIN32)
  set(CPACK_GENERATOR "DEB;NuGet;TGZ;ZIP")
else()
  set(CPACK_GENERATOR "ZIP")
endif()

set(CPACK_DEBIAN_PACKAGE_DEPENDS " ")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE_URL "https://github.com/ArthurSonzogni/rgb-tui/")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arthur Sonzogni")
set(CPACK_DEBIAN_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A color picker with a terminal UI.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/ArthurSonzogni/rgb-tui/")
set(CPACK_PACKAGE_NAME "rgb-tui")
set(CPACK_PACKAGE_VENDOR "Arthur Sonzogni")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include(CPack)
