# adafruit_ft232h_comm
Serial communication using the USB-to-Serial FT232H chip on the Adafruit breakout board

# Requirements

- [libmpsse](https://github.com/devttys0/libmpsse)
- [libftdi1](https://www.intra2net.com/en/developer/libftdi/repository.php)

# Installation

## External Dependencies

### MPSSE

```bash
$ git clone https://github.com/devttys0/libmpsse
$ cd libmpsse/src
$ <edit Makefile install>
$ ./configure --disable-python
$ make -j4
$ make install
```
where the `<edit Makefile install>` is for installation on `OSX` and refers to removing the `-D` option to the `install` commands. From this:
```bash
install: py$(BUILD)-install
    install -D -m644 lib$(TARGET).so ...
    install -D -m644 lib$(TARGET).a ...
    install -D -m644 $(TARGET).h ...
```
to
```bash
install: py$(BUILD)-install
    install -D -m644 lib$(TARGET).so ...
    install -D -m644 lib$(TARGET).a ...
    install -D -m644 $(TARGET).h ...
```

By default, this installs to `/usr/local/`:
   * `/usr/local/lib/libmpsse.so`
   * `/usr/local/include/mpsse.h`

### LIBFTDI

#### Linux

Follow the instructions under `libftdi/README.build`.

#### OSX

```bash
$ brew install libftdi
```

Which provides a link to:
   * `/usr/local/lib/libftdi1.dylib`
   * `/usr/local/include/libftdi1/ftdi.h`
