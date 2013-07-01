# Blynux

Blync for Linux

The device VID:PID are 0x1130:0x0001.

There are 6 colors plus an off command:
- WHITE
- CYAN
- MAGENTA
- BLUE
- YELLOW
- GREEN
- RED
- OFF

The data control payload is { 0x55, 0x53, 0x42, 0x43, 0x00, 0x40, 0x02, 0x0f};

## How to compile

You need
* `cmake >= 2.8` with `FindPkgConfig.cmake` module
* `libusb-1.0-0-dev` package


After checking out/cloning the code:

    cd blynux
    mkdir build
    cd build
    cmake ..
    make
  
