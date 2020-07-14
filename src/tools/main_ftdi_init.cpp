//std/stl
#include <iostream>
#include <string>
#include <memory>

//ftdi
#include "libftdi1/ftdi.h"

// device info
#define VENDOR_ID 0x403
#define DEVICE_ID 0x6014

// comm info
#define CHUNK_SIZE 65535 // 64 kilobit
#define LATENCY_MS 4
#define USB_TIMEOUT 120000

struct ftdi_context *m_ftdi;

int connect_to_device()
{
    if((m_ftdi = ftdi_new()) == 0)
    {
        std::cout << "ERROR: Could not create new FTDI context" << std::endl;
        return -1;
    }

    int ret = ftdi_usb_open(m_ftdi, VENDOR_ID, DEVICE_ID);
    if(ret < 0)
    {
        std::cout << "ERROR: Could not locate FT232H device (err: " << ret << ")" << std::endl;
        ftdi_free(m_ftdi);
        return -1;
    }

    std::cout << "FT232H device initialized: 0x"<<std::hex << VENDOR_ID << ":0x"<<DEVICE_ID << std::dec << std::endl;
    return 0;
}

int configure_device()
{
	// reset USB device
	ftdi_usb_reset(m_ftdi);

	// configure USB block transfer sizes
	ftdi_write_data_set_chunksize(m_ftdi, CHUNK_SIZE);
	ftdi_read_data_set_chunksize(m_ftdi, CHUNK_SIZE);

	// disable event and error characters
	ftdi_set_event_char(m_ftdi, 0, false);
	ftdi_set_error_char(m_ftdi, 0, false);

	// set read and write timeouts (ms)
	m_ftdi->usb_read_timeout = USB_TIMEOUT;
	m_ftdi->usb_write_timeout = USB_TIMEOUT;

	// set the latency timer (default: 1ms)
	ftdi_set_latency_timer(m_ftdi, LATENCY_MS);

	
}

int main(int argc, char* argv[])
{
    int ret = connect_to_device();
    if(ret<0)
    {
        return -1;
    }

	//
	// configure USB and MPSSE
	//
	ret = configure_device();


    //
    // finished
    //
    ftdi_usb_close(m_ftdi);
    ftdi_free(m_ftdi);

    return 0;
}
