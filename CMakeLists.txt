cmake_minimum_required (VERSION 3.11)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 9d15d1c275726d21081ef6912ff57f99f1c348d3
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

install(TARGETS rgb-tui RUNTIME DESTINATION "bin")
