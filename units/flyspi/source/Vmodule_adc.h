#pragma once
#include "vmsp6.h"
#include <bitset>
#include "Vocpmodule.h"

class Vflyspi_adc : protected Vocpmodule {

	public:
		Vflyspi_adc(Vmodule<sp6adr,sp6data> *parent):Vocpmodule(parent,0x3000) {};

		struct Status {
			uint32_t start_addr;
			uint32_t end_addr;
			// Read from internal state, if trigger is active
			bool trigger_enabled_bit ;
			// Is set after the trigger has triggered
			bool triggered_bit ;
		};

		//HAL
//adc configuration functions
		void configure(unsigned int test_pattern);
		void configure(std::bitset<3> test_pattern);
		void run();
		void off();

//trigger configuration functions
		uint32_t get_startaddr();
		uint32_t get_endaddr();
        void set_addr(sp6data *buf, uint32_t startaddr, uint32_t endaddr);
		void setup_controller(uint32_t startaddr, uint32_t endaddr, bool singlemode,bool trigger_enabled, bool trigger_channel=0);

// WARNING: use these for debug only
		uint32_t get_trigger_status();
		Status get_status();
		void log_status();

//trigger event functions
		void set_single_trigger();
		void set_off_trigger();
		void manual_trigger();

		uint32_t get_version();

// functions to be inherited by derived classes
	//write to the configuration register
		using Vocpmodule::write;


	private:
//adress of configuration register
		static const unsigned int ocpwrite = 0x80000000;

//numbering for adc registers
		enum adcregs {startadr_msb,startadr_2,startadr_1,startadr_lsb,endadr_msb,endadr_2,endadr_1,endadr_lsb,config,startstop,cmdfull,commitid};

//values for starting and stopping manual recording						
		enum start_m {start_val=1,stop_val=2};
//ocp package function
		inline void writeBuf(sp6data * & buf ,uint adr, uint data);
};

