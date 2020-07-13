//std/stl
#include <iostream>
#include <string>
#include <memory>

//ftdi
#include "libftdi1/ftdi.h"

#define VENDOR_ID 0x403
#define DEVICE_ID 0x6014

int main(int argc, char* argv[])
{
    std::cout << "ftdi_init" << std::endl;

    struct ftdi_context* ftdi;
    int ret;

    //
    // open and connect FTDI context to FT232H
    //
    if((ftdi = ftdi_new()) == 0)
    {
        std::cout << "ERROR: Could not create new FTDI context" << std::endl;
    }

    (ret = ftdi_usb_open(ftdi, VENDOR_ID, DEVICE_ID));
    if(ret<0)
    {
        std::cout << "ERROR: Could no locate FT232H device (error: " << ret << ")" << std::endl;
    }

    std::cout << "FTDI device initialized: VID = 0x" << std::hex << VENDOR_ID << ", PID = 0x" << DEVICE_ID << std::dec << std::endl;


    //
    // finished
    //
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);

    return 0;
}
