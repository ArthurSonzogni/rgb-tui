cmake_minimum_required (VERSION 3.11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 7b1f4d435bedecc49abc3f935bd97dc997a41977
)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 2cac8a9
)

FetchContent_MakeAvailable(ftxui)

if(NOT fmt_POPULATED)
  message(STATUS "populate fmt > Start")
  FetchContent_Populate(fmt)
  set(BUILD_SHARED_LIBS FALSE)
  message(STATUS "populate fmt > Done")
  add_subdirectory(
    ${fmt_SOURCE_DIR}
    ${fmt_BINARY_DIR}
    EXCLUDE_FROM_ALL)
endif()

# ------------------------------------------------------------------------------

project(rgb-tui
  LANGUAGES CXX
  VERSION 1.0.0
)

add_executable(rgb-tui src/main.cpp)
target_include_directories(rgb-tui PRIVATE src)

target_link_libraries(rgb-tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component # Not needed for this example.
  PRIVATE fmt::fmt
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
