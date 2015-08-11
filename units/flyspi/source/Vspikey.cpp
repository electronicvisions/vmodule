/*
	vmodule classes for Spikey on sp6board
	Andreas Gruebl 2012
*/


// new electronic visions slow-control base class

using namespace std;

#include <cstring>
#include <list>
#include <bitset>

#include "logger.h"

#include "vmodule.h"
#include "vmsp6.h"
#include "Vspikey.h"

static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("vmodule.vspikey");

float Vspikeyslowadc::readVoltage(uint channel) {
	std::bitset<12> adc_value;

	if(boardVersion == 1){
		// acquire raw data with two ocp reads
		std::bitset<16> raw_data(read(0x100)<<8);
		raw_data |= std::bitset<16>(read(0x900) & 0xff);
		//std::cout << raw_data.to_string() << std::endl;

		// raw_data has two leading and trailing zeros
		adc_value = (raw_data>>2).to_ulong()&0xfff;
	}
	else if(boardVersion == 2){
		assert(channel < 2);

		uint channel_select = 0x080; //channel == 0
		if(channel == 1){
			channel_select = 0x0c0;
		}

		// setup adc for next conversion:
		write(channel_select, 0xc0 ); //TP 2015/04/30: second argument may be unnecessary
		write(0x800, 0 );

		// acquire raw data with two ocp reads
		// TP 2015/04/30: addresses in the following reads may be unnecessary
		std::bitset<16> raw_data((read(channel_select)&0xff) << 8);
		raw_data |= std::bitset<16>(read(0x800) & 0xff);
		//std::cout << raw_data.to_string() << std::endl;

		// raw_data has two leading and trailing zeros
		adc_value = (raw_data>>4).to_ulong()&0xfff; // >> 4: 12bit adc, lower 4bit not used.
	}
	else {
		std::string msg = "Board version " + std::to_string(boardVersion) + " not supported";
		throw std::logic_error(msg);
	}

	float voltage = float(adc_value.to_ulong())/4095.*2.5;
	LOG4CXX_DEBUG(logger, "Slow ADC (channel " << channel << "): " << dec << voltage << "V");

	return voltage;
};

void Vspikeyfastadc::setup_controller(uint32_t startaddr, uint32_t endaddr) {
    //write configuration
    sp6data *buf=writeBlock(0,16);
    set_addr(buf, startaddr, endaddr);
    
    doWB();
};
