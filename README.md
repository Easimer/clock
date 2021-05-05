# clock

[![Build Status](https://ci.easimer.net/job/clock/badge/icon)](https://ci.easimer.net/job/clock/) ![Test coverage](https://img.shields.io/jenkins/coverage/cobertura.svg?jobUrl=https%3A%2F%2Fci.easimer.net%2Fjob%2Fclock)

*clock* is a ATmega2560-based digital clock.

## Features:
- Current time is regularly saved to an external EEPROM chip (PCF8594C-2)
- A High Endurance module implementing wear leveling for the external EEPROM
- High-level input system so one physical button can be used for multiple actions
- BCD to 7-segment decoding and digit multiplexing
- Most modules are hardware-independent and unit tested

## Planned features:
- Stopwatch
- Reduce number of used pins by using an external BCD decoder
- Support for Nokia 5110 displays

## Building
To build the Arduino executable, simply open the repository in the Arduino IDE and compile as usual.

To build and run the tests on a PC use CMake to generate the VS solution/Makefiles:
```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```
Then either run CTest or the test executable directly:
```
$ ctest -C Debug
OR
$ ./clock_tests
```

## Misc

The EEPROM HE module implements the algorithm described in "[AVR101: High Endurance EEPROM Storage](http://ww1.microchip.com/downloads/en/appnotes/doc2526.pdf)".

Pin assignments are reconfigurable in `config.h`.

We're using the [utest.h](https://github.com/sheredom/utest.h) unit testing library.