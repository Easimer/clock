cmake_minimum_required(VERSION 3.10)
project(clock)
enable_testing()

set(C_STANDARD 90)
set(C_STANDARD_REQUIRED ON)
set(C_EXTENSIONS OFF)

set(SRC_CORE
  eeprom_he.c
  eeprom_he.h
)

set(SRC_TESTS
  eeprom_he.tests.c
)

add_library(clock_core ${SRC_CORE})

add_executable(clock_tests ${SRC_TESTS})
target_link_libraries(clock_tests PRIVATE clock_core )

add_test(NAME clock_tests COMMAND clock_tests)