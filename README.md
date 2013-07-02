# Blynux

Blync for Linux

The device VID:PID are 0x1130:0x0001.

There are 7 colors plus an off command:
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

### Install

    make install

### Uninstall

    xargs rm < install_manifest.txt
  
## How to use it

If you want to use your device in user mode, you need to install this udev rules inside `/etc/udev/rules.d/10-blync.rules`

    SUBSYSTEM=="input", GROUP="input", MODE="0666"
    SUBSYSTEM=="usb", ATTRS{idVendor}=="1130", ATTRS{idProduct}=="0001", MODE:="666", GROUP="plugdev"
    
## License

BSD 3-Clause

