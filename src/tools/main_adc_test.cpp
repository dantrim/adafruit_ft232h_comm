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
#include "support.h"
}

////ftdi
//#include "libftdi1/ftdi.h"

// device info
#define VENDOR_ID 0x403
#define DEVICE_ID 0x6014

// comm info
#define CHUNK_SIZE 65535 // 64 kilobit
//#define LATENCY_MS 4
#define USB_TIMEOUT 120000

#define FTDI_VERSION FTDI_MAJOR_VERSION.

struct mpsse_context* m_i2c = 0;//= MPSSE(I2C, ONE_HUNDRED_KHZ, MSB);

#define PIN_CONV 4
#define PIN_ALERT 5
#define PIN_EFUSE_EN 6
#define PIN_TRIM0_EN 7
#define PIN_TRIM1_EN 8
#define PIN_TRIM2_EN 9
#define PIN_TRIM3_EN 10
#define PIN_VDD_SHUNT_EN 11

class I2CDevice
{
    public :
        I2CDevice(){ m_i2c = nullptr; };
        bool open()
        {
	        if((m_i2c = MPSSE(I2C, FOUR_HUNDRED_KHZ,MSB)) == NULL || !m_i2c->open)
	        {
	        	std::cout << "ERROR: Unable to create MPSSE device" << std::endl;
                m_i2c = nullptr;
	        	return false;
	        }
            return true;
        }

        struct mpsse_context* bus() { return m_i2c; }

        ~I2CDevice()
        {
            Close(m_i2c);
        }

    private :
        struct mpsse_context* m_i2c;
};

class ADC
{
    public :
        ADC(std::shared_ptr<I2CDevice> i2c, uint8_t address)
        {
            m_address = address;
            m_i2c = i2c;
        }
        ~ADC(){};

        struct mpsse_context* i2c() { return m_i2c->bus(); };

        bool check()
        {
            bool found = false;
            uint8_t shifted_address = (m_address << 1);
            std::vector<uint8_t> data = { shifted_address };

            Start(i2c());
            Write(i2c(), const_cast<char*>(reinterpret_cast<const char*>(&data[0])), 1);
            found = (GetAck(i2c()) == ACK);
            Stop(i2c());
            return found; 
        }

        void start_conv()
        {
            auto ctx = this->i2c();
            PinLow(ctx, PIN_CONV);
            std::this_thread::sleep_for(std::chrono::microseconds(5));
            PinHigh(ctx, PIN_CONV);
            std::this_thread::sleep_for(std::chrono::microseconds(5));
            PinLow(ctx, PIN_CONV);
        }

        void read()
        {
            Start(i2c());

            uint8_t shifted_address = (m_address << 1);
            std::vector<uint8_t> data = { shifted_address };
            Write(i2c(), const_cast<char*>(reinterpret_cast<const char*>(&data[0])), 1);

            if(GetAck(i2c()) == ACK)
            {
                char* data = Read(i2c(), 2);
                if(data)
                {
                    std::cout << "READ: 0x:" << std::hex << (unsigned)data[0] << " " << (unsigned)data[1] << std::endl;
                }
                free(data);
                SendNacks(i2c());
                Read(i2c(), 1);
            }

            Stop(i2c());
        }

    private :

        uint8_t m_address;
        std::shared_ptr<I2CDevice> m_i2c;
};

void find_devices(struct mpsse_context* i2c)
{
    std::vector<uint8_t> lookup_addresses = {
        0x21 /*010_0001 AD7997/8-0 AS=GND*/
        ,0x22 /*010_0010 AD7997/8-0 AS=VDD*/
        ,0x23 /*010_0011 AD7997/8-1 AS=GND*/
        ,0x24 /*010_0100 AD7997/8-1 AS=VDD*/
        ,0x20 /*010_0000 AD7997/8-x AS=FLOAT*/
    };


    for(auto address : lookup_addresses)
    {
        std::cout << "Checking address: 0x" << std::hex << (unsigned)address << std::dec << std::endl;
        //
        // Send I2C START
        //
        Start(i2c);

        uint8_t address_check = (address << 1);


        //
        // Write address
        //
        std::vector<uint8_t> data = { address_check };
        Write(i2c, const_cast<char*>(reinterpret_cast<const char*>(&data[0])), data.size());

        //
        // check if a device returned ACK
        //
        if(GetAck(i2c) == ACK)
        {
            std::cout << "--> Received ACK from address: 0x" << std::hex << (unsigned)address << std::dec << std::endl;
        }

        //
        // Send I2C STOP
        //
        Stop(i2c);
    }

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
    int pin_set = std::atoi(argv[1]);
    std::shared_ptr<I2CDevice> i2c = std::make_shared<I2CDevice>();
    if(!i2c->open())
    {
        std::cout << "ERROR Failed to open I2C bus" << std::endl;
        return -1;
    }
    std::cout << "I2C bus open" << std::endl;


    std::unique_ptr<ADC> adcU2 = std::make_unique<ADC>(i2c, 0x21);
    std::unique_ptr<ADC> adcU3 = std::make_unique<ADC>(i2c, 0x22);

    bool u2_found = adcU2->check();
    bool u3_found = adcU3->check();

    std::cout << "ADC U2 found: " << (u2_found ? "yes" : "no") << std::endl;
    std::cout << "ADC U3 found: " << (u3_found ? "yes" : "no") << std::endl;

    std::cout << "starting conv on U2" << std::endl;
    adcU2->start_conv();
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    adcU2->read();


//    auto ctx = i2c->bus();
//    PinLow(ctx, pin_set);
//    printf("GPIOL %d state: %d\n", pin_set, PinState(ctx, pin_set, -1));
//    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//    PinHigh(ctx, pin_set);
//    printf("GPIOL %d state: %d\n", pin_set, PinState(ctx, pin_set, -1));
//    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//    PinLow(ctx, pin_set);
//    printf("GPIOL %d state: %d\n", pin_set, PinState(ctx, pin_set, -1));
//    //int ret = set_bits_low(ctx, 0x0f);
//    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    

    
    

	//if((m_i2c = MPSSE(I2C, ONE_HUNDRED_KHZ,MSB)) == NULL || !m_i2c->open)
	//{
	//	std::cout << "ERROR: Unable to create MPSSE device" << std::endl;
	//	return -1;
	//}
	////Tristate(m_i2c);

    //find_devices(m_i2c);

//	if(write_block(m_i2c) < 0)
//	{
//		std::cout << "ERROR: Unable to write block" << std::endl;
//		return -1;
//	}

    std::cout << "done" << std::endl;
    return 0;


}
