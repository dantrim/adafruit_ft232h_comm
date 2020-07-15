//std/stl
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>

extern "C"{
#include <mpsse.h>
}

//ftdi
#include "libftdi1/ftdi.h"

// device info
#define VENDOR_ID 0x403
#define DEVICE_ID 0x6014

// comm info
#define CHUNK_SIZE 65535 // 64 kilobit
//#define LATENCY_MS 4
#define USB_TIMEOUT 120000

#define FTDI_VERSION FTDI_MAJOR_VERSION.

struct ftdi_context *m_ftdi;

float ftdi_version()
{
	auto vs = ftdi_get_library_version();
	int major = static_cast<int>(vs.major);
	int minor = static_cast<int>(vs.minor);
	std::stringstream version;
	version << major << "." << minor;
	return std::stof(version.str());
}

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

int read_data(std::vector<uint8_t>& data, uint32_t requested)
{

	data.resize(requested);
	uint32_t count = 0;
	for(uint8_t try_num = 0; try_num < 10; try_num++)
	{
		int32_t ret = ftdi_read_data(m_ftdi, &data[count], data.size() - count);
		if(ret < 0)
		{
			return ret;
		}
		count += ret;
		if(count == requested)
		{
			return count;
		}

		// give the hw some time
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	} // try_num
	return count;
}

bool mpsse_is_enabled()
{

	//
	// If MPSSE is correctly enabled, if a bad command is detected by MPSSE
	// it will return the value of 0xFA to the host followed by the byte
	// that caused the bad command (see Section 4.3 of AN_135
	// https://www.ftdichip.com/Support/Documents/AppNotes/AN_135_MPSSE_Basics.pdf).
	// AN_108 (https://www.ftdichip.com/Support/Documents/AppNotes/AN_108_Command_Processor_for_MPSSE_and_MCU_Host_Bus_Emulation_Modes.pdf)
	// contains a description of all accepted MPSSE commands.
	//
	uint8_t bad_cmd = 0xAA; /// 0xAA is not a valid MPSSE command

	std::vector<uint8_t> data_send = { bad_cmd }; 
	ftdi_write_data(m_ftdi, &data_send[0], data_send.size());
	std::cout << "SENT: ";
	for(auto b : data_send)
	{
		std::cout << "0x" << std::hex << (unsigned)b << " " << std::dec;
	}
	std::cout << "\n";

	std::vector<uint8_t> data_recv;
	int ret = read_data(data_recv, data_send.size() + 1); // we expect 0xFA 0xAA
	if(ret < 0)
	{
		std::cout << "ERROR: Failed to read data" << std::endl;
		return false;
	}
	std::cout << "RECV: ";
	for(auto b : data_recv)
	{
		std::cout << "0x" << std::hex << (unsigned)b << " " << std::dec;
	}
	std::cout << "\n";

	if(ret != (data_send.size()+1))
	{
		std::cout << "ERROR: Did not receive expected amount of data (n_sent: " << data_send.size() << ", n_recv: " << data_recv.size() << ", n_recv_expected: " << data_send.size() + 1 << ")" << std::endl;
		return false;
	}

	uint8_t returned_cmd = data_recv.at(0);
	uint8_t bad_sent_cmd = data_recv.at(1);
	if(returned_cmd == 0xFA && bad_sent_cmd == bad_cmd)
	{
		return true;
	}
	return false;
}

void set_i2c_idle()
{
	std::cout << "\n*** INFO: Setting I2C IDLE state ***\n" << std::endl;

	std::vector<uint8_t> data;

	// initialize MPSSE low byte register
	// See Section 3.6.1 of AN_108 (https://www.ftdichip.com/Support/Documents/AppNotes/AN_108_Command_Processor_for_MPSSE_and_MCU_Host_Bus_Emulation_Modes.pdf)
	// Pin name		Signal		Direction		Config		Initial State		Config
	// ADBUS0		TCK/SK		OUTPUT			1			HIGH				1
	// ADBUS1		TDI/DO		OUTPUT			1			LOW					0
	// ADBUS2		TDO/DI		INPUT			0								0
	// ADBUS3		TMS/CS		INPUT			0								0				
	// ADBUS4		GPIOL0		INPUT			0								0	
	// ADBUS5		GPIOL1		INPUT			0								0	
	// ADBUS6		GPIOL2		INPUT			0								0	
	// ADBUS7		GPIOL3		OUTPUT			1			HIGH				1
	data = { 0x80, 0xff /*value mask*/, 0xfb /*direction mask*/ };
	ftdi_write_data(m_ftdi, &data[0], data.size());

	// initialize MPSSE high byte register
	// Pin name		Signal		Direction		Config		Initial State		Config
	// ACBUS0		GPIOH0	    INPUT			0								0	
	// ACBUS1		GPIOH1	    INPUT			0								0
	// ACBUS2		GPIOH2	    INPUT  		    0								0	
	// ACBUS3		GPIOH3	    INPUT			0								0	
	// ACBUS4		GPIOH4	    INPUT			0								0	
	// ACBUS5		GPIOH5	    INPUT			0								0	
	// ACBUS6		GPIOH6	    INPUT			0								0	
	// ACBUS7		GPIOH7	    INPUT			0								0	
	data = { 0x82, 0xff/*value mask*/, 0x40 /*direction mask*/};
	ftdi_write_data(m_ftdi, &data[0], data.size());

}

void i2c_start()
{
	std::vector<uint8_t> data;
	// SDA low, SCA high
	for(size_t i = 0; i < 100; i++)
	{
		data.push_back(0x80);
		data.push_back(0xfd);
		data.push_back(0xfb);
	}

	//ftdi_write_data(m_ftdi, &data[0], data.size());
	//std::this_thread::sleep_for(std::chrono::microseconds(100));
	//data.clear();

	// SDA low, SCA low
	for(size_t i = 0; i < 100; i++)
	{
		data.push_back(0x80);
		data.push_back(0xfc);
		data.push_back(0xfb);
	}
	ftdi_write_data(m_ftdi, &data[0], data.size());
	//std::this_thread::sleep_for(std::chrono::microseconds(100));
	data.clear();


	data.push_back(0x82);
	data.push_back(0x40);
	data.push_back(0x40);

	ftdi_write_data(m_ftdi, &data[0], data.size());
}

void i2c_stop()
{
	std::vector<uint8_t> data;

	// SDA low, SCL low
	for(size_t i = 0; i < 4; i++)
	{
		data.push_back(0x80);
		data.push_back(0xfc);
		data.push_back(0xfb);
	}
	//ftdi_write_data(m_ftdi, &data[0], data.size());
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//data.clear();

	// SDA low, SCL high
	for(size_t i = 0; i < 4; i++)
	{
		data.push_back(0x80);
		data.push_back(0xfd);
		data.push_back(0xfb);
	}
	//ftdi_write_data(m_ftdi, &data[0], data.size());
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//data.clear();

	// SDA high, SCL high
	for(size_t i = 0; i < 4; i++)
	{
		data.push_back(0x80);
		data.push_back(0xff);
		data.push_back(0xfb);
	}
	//ftdi_write_data(m_ftdi, &data[0], data.size());
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//data.clear();

	data.push_back(0x82);
	data.push_back(0xff);
	data.push_back(0x40);

	ftdi_write_data(m_ftdi, &data[0], data.size());

}

int configure_device()
{
	//
	// Configures FTDI port for MPSSE use.
	// See Section 4.2 and Section 5 of FTDI AN_135 for complete description
 	// (https://www.ftdichip.com/Support/Documents/AppNotes/AN_135_MPSSE_Basics.pdf)
	//

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
	// (amount of time to wait before sending an incomplete USB packet from
	// the peripheral back to the hose PC)
	ftdi_set_latency_timer(m_ftdi, LATENCY_MS);

	// configure flow control to ensure that the driver will not issue
	// IN requests if the buffer is unable to accept data
	// RTS/CTS RS-232 control signals
	// RTS: request to send (FTDI checks with peripheral)
	// CTS: clear to send (peripheral raises)
	ftdi_setflowctrl(m_ftdi, SIO_RTS_CTS_HS);

	// reset the controller
	// mask = 0 && mode = 0 resets the MPSSE controller
	ftdi_set_bitmode(m_ftdi, 0x0 /*direction mask*/, BITMODE_RESET /*mode*/);

	// enable the MPSEE controller
	// mask = 0 && mode = 0x2 enables MPSEE controller 
	// (pin directions are then set via direct MPSSE commands)
	ftdi_set_bitmode(m_ftdi, 0x2, BITMODE_MPSSE);

	// clear tx/rx buffers
	int ret = 0;
	float libftdi_version = ftdi_version();
	std::cout << "libftdi version: " << libftdi_version << std::endl;
	if(ftdi_version() < 1.5)
	{
		ret = ftdi_usb_purge_buffers(m_ftdi);
	}
	else
	{
		ret = ftdi_tcioflush(m_ftdi);
	}
	if(ret < 0)
	{
		std::cout << "ERROR: Failed to purge USB RX/TX buffers (error: " << ret << ")" << std::endl;
		return -1;
	}

	//
	// ** MPSSE is now enabled and we can communicate with MPSSE controller **
	//
	if(!mpsse_is_enabled())
	{
		return -1;
	}

	//
	// configure MPSSE
	// See Section 4.3 of AN_135 (https://www.ftdichip.com/Support/Documents/AppNotes/AN_135_MPSSE_Basics.pdf)
	//
//	std::vector<uint8_t> data;
//
//
//	// disable adaptive clocking (required only for JTAG interface)
//	data = { 0x97 };
//	ftdi_write_data(m_ftdi, &data[0], data.size());
//
//	// enable 3-phase data clocking (required for I2C)
//	// (see Section 2.2.4 of AN_255 https://www.ftdichip.com/Support/Documents/AppNotes/AN_255_USB%20to%20I2C%20Example%20using%20the%20FT232H%20and%20FT201X%20devices.pdf)
//	std::cout << "\n*** INFO: Assuming an I2C interface and enabling three-phase data clocking!***\n" << std::endl;
//	data = { 0x8c };
//	ftdi_write_data(m_ftdi, &data[0], data.size());
//
//	// disable 5x clock divisor to get higher rate
//	// Section 3.8.2 of AN_108 (https://www.ftdichip.com/Support/Documents/AppNotes/AN_108_Command_Processor_for_MPSSE_and_MCU_Host_Bus_Emulation_Modes.pdf)
//	data = { 0x8a };
//	ftdi_write_data(m_ftdi, &data[0], data.size());
//
//	// enable drive-zero mode
//	data = { 0x9e, 0x07, 0x00 };
//	ftdi_write_data(m_ftdi, &data[0], data.size());
//
//	// ensure loopback is off
//	data = { 0x85 };
//	ftdi_write_data(m_ftdi, &data[0], data.size());
//
//	// set clock frequency to max 30MHz
//	// TCK period (5x divisor off) = 60 MHz / (( 1 + [ (value_high * 256) OR value_low]) * 2)
//	uint16_t clock_divisor = 0xc8;
//	data = { 0x86, static_cast<uint8_t>(clock_divisor & 0xff) /*value low*/, static_cast<uint8_t>((clock_divisor >> 8) & 0xff) /*value high*/};


	return 0;
}

int write_block(struct mpsse_context* i2c)
{
	if(Start(i2c)!=0)
	{
		std::cout << "ERROR: write_block failed on Start" << std::endl;
		return -1;
	}

	uint8_t device_address = 0x3;
	uint8_t register_address = 0xca;

	std::vector<uint8_t> data;
	data = { static_cast<uint8_t>(device_address << 1), static_cast<uint8_t>(register_address & 0xff) };
	if(Write(i2c, const_cast<char*>(reinterpret_cast<const char*>(&data[0])), 2) != 0)
	{
		std::cout << "ERROR: write_block failed on Write device_address" << std::endl;
		return -1;
	}

	//data = { static_cast<uint8_t>(register_address & 0xff) };
	//if(Write(i2c, const_cast<char*>(reinterpret_cast<const char*>(&data[0])), 1) != 0)
	//{
	//	std::cout << "ERROR: write_block failed on Write register_address" << std::endl;
	//	return -1;
	//}

	if(Stop(i2c) != 0)
	{
		std::cout << "ERROR: write_block failed on Stope" << std::endl;
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
//    int ret = connect_to_device();
//    if(ret<0)
//    {
//        return -1;
//    }
//
//	//
//	// configure USB and MPSSE
//	//
//	ret = configure_device();

	struct mpsse_context* i2c = 0;//= MPSSE(I2C, ONE_HUNDRED_KHZ, MSB);
	if((i2c = MPSSE(I2C, ONE_HUNDRED_KHZ,MSB)) == NULL || !i2c->open)
	{
		std::cout << "ERROR: Unable to create MPSSE device" << std::endl;
		return -1;
	}
	Tristate(i2c);

	if(write_block(i2c) < 0)
	{
		std::cout << "ERROR: Unable to write block" << std::endl;
		return -1;
	}

	//std::cout << "i2c idle" << std::endl;

	// initialize idle state
//	set_i2c_idle();
//	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//
//	std::cout << "calling start" << std::endl;
//	i2c_start();
//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	std::cout << "calling stop" << std::endl;
//	i2c_stop();


    //
    // finished
    //
//	ftdi_set_bitmode(m_ftdi, 0x00, BITMODE_RESET);
//    ftdi_usb_close(m_ftdi);
//	ftdi_deinit(m_ftdi);
//    ftdi_free(m_ftdi);

    return 0;
}
