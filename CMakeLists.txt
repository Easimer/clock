cmake_minimum_required(VERSION 3.10)
project(clock)
enable_testing()

set(C_STANDARD 90)
set(C_STANDARD_REQUIRED ON)
set(C_EXTENSIONS OFF)

set(SRC_CORE
  config.h

  7seg.c
  7seg.h
  actions.c
  actions.h
  core.c
  core.h
  display.c
  display.h
  display_7seg.c
  display_7seg.h
  eeprom_access.h
  eeprom_he.c
  eeprom_he.h
  kprintf.c
  kprintf.h
  timekeeper.c
  timekeeper.h
  timer.c
  timer.h
  timesave_io.c
  timesave_io.h
)

set(SRC_TESTS
  tests/actions.tests.c
  tests/display.tests.c
  tests/eeprom_he.tests.c
  tests/timekeeper.tests.c
)

add_library(clock_core ${SRC_CORE})
target_include_directories(clock_core PUBLIC .)

add_executable(clock_tests ${SRC_TESTS})
target_link_libraries(clock_tests PRIVATE clock_core )

add_test(NAME clock_tests COMMAND clock_tests)
