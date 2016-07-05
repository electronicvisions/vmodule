#include "Vmodule_adc.h"

uint16_t const Vflyspi_adc::adc_sample_size = 12; // bits
uint16_t const Vflyspi_adc::adc_sample_mask = (1 << adc_sample_size) - 1;
uint32_t const Vflyspi_adc::mem_read_offset = 0x08000000;
size_t const Vflyspi_adc::samples_per_word = sizeof(sp6_word_t) / sizeof(adc_sample_t);

//*** helper to fill buffer

inline void Vflyspi_adc::writeBuf(sp6data * & buf ,uint adr, uint data){
	*buf++=(adr+baseadr+ocpwrite);
	*buf++=(data);
}


//*** adc configuration functions

	void Vflyspi_adc::configure( unsigned int test_pattern ) {
		configure(static_cast<std::bitset<3>>(test_pattern));
	}
//set operation mode of adc
	void Vflyspi_adc::configure( std::bitset<3> test_pattern ) {
	    // Testpatterns are:
	    // 000 Normal operation - <D13:D0> = ADC output
	    // 001 All zeros - <D13:D0> = 0x0000
	    // 010 All ones - <D13:D0> = 0x3FFF
	    // 011 Toggle pattern - <D13:D0> toggles between 0x2AAA and 0x1555
	    // 100 Digital ramp - <D13:D0> increments from 0x0000 to 0x3FFF by one code every cycle
	    // 101 Custom pattern - <D13:D0> = contents of CUSTOM PATTERN registers
	    // 110 Unused
	    // 111 Unused

	    Logger& log = Logger::instance();
	    log(Logger::DEBUG0) << "Setting ADC input to: " << test_pattern.to_string();

	    sp6data* buf = writeBlock(0,18);//just a large one
	    *(buf++) = ocpwrite | 0x3008; *(buf++) = 0x08; //start clock
	    *(buf++) = ocpwrite | 0x3008; *(buf++) = 0xe8; //release pdn and reset for adc, no start signal yet
	    *(buf++) = ocpwrite | 0x1c00; *(buf++) = 0x10; //software reset
	    *(buf++) = ocpwrite | 0x1c01; *(buf++) = 0x4; //enable lvds interface, disable clock
	//  ocp.write(0x1c00|(0xe<<3)|2,0x81); //7mA, 100Ohm
	    *(buf++) = ocpwrite | 0x1c00|(0xe<<3); *(buf++) = 0xc; //lowest power lvds is 0xc
	    *(buf++) = ocpwrite | 0x1c00|(0x4<<3)|0; *(buf++) = 0x0; //clock edge
	    *(buf++) = ocpwrite | 0x1c00|(0xb<<3)|1; *(buf++) = 0x54; //custom pattern 0x555 ->datablatt wrong!!!
	    *(buf++) = ocpwrite | 0x1c00|(0xc<<3)|0; *(buf++) = 0x05; //custom pattern 0x555 ->datablatt wrong!!!
	    *(buf++) = ocpwrite | 0x1c00|(0xa<<3)|4; *buf =test_pattern.to_ulong()<<5; //enable ramp test pattern with 0x80, custom with 0xa0
	    doWB();
	}

//switch off the adc	
	void Vflyspi_adc::off() {
		    write(0x0008,0x0); //switch off adc
		}

//get start address for writing into adc board ram
	uint32_t Vflyspi_adc::get_startaddr() {
		uint32_t startaddr = 0;
		startaddr |= (read(startadr_msb)&0xff)<<24;
		startaddr |= (read(startadr_2)&0xff)<<16;
		startaddr |= (read(startadr_1)&0xff)<<8;
		startaddr |= (read(startadr_lsb)&0xff)<<0;
		return startaddr;
	}
	

//get end address for writing into adc board ram
	uint32_t Vflyspi_adc::get_endaddr() {
		uint32_t endaddr = 0;
		endaddr |= (read(endadr_msb)&0xff)<<24;
		endaddr |= (read(endadr_2)&0xff)<<16;
		endaddr |= (read(endadr_1)&0xff)<<8;
		endaddr |= (read(endadr_lsb)&0xff)<<0;
		return endaddr;
	}
//get trigger config register
	uint32_t Vflyspi_adc::get_trigger_status(){
		uint32_t trigger_status = read(startstop&0xff);
		return trigger_status;
	}

	Vflyspi_adc::Status Vflyspi_adc::get_status(){
		Status st;

		uint32_t trigger_status=Vflyspi_adc::get_trigger_status();
		//get startaddr and endaddr
		st.start_addr=Vflyspi_adc::get_startaddr();
		st.end_addr =Vflyspi_adc::get_endaddr();

		// Read from internal state, if trigger is active
		st.trigger_enabled_bit = (trigger_status>>5) & 0x1;
		// Is set after the trigger has triggered
		st.triggered_bit = (trigger_status>>6) & 0x1;
		return st;
	}


//use this method to log status of adc
	void Vflyspi_adc::log_status(){
//get startaddr and endaddr
		uint32_t startaddr=Vflyspi_adc::get_startaddr();
		uint32_t endaddr =Vflyspi_adc::get_endaddr();
//get trigger_status		
		uint32_t trigger_status=Vflyspi_adc::get_trigger_status();
		bool single_mode_bit=(trigger_status>>2) & 0x1;
		bool trigger_enabled_bit=(trigger_status>>3) & 0x1;
		bool trigger_channel_bit=(trigger_status>>4) & 0x1;
		bool triggerenabled_bit = (trigger_status>>5) & 0x1;
		bool triggered_bit = (trigger_status>>6) & 0x1;
//do some logging
		Logger& log = Logger::instance();
		log(Logger::DEBUG0) << "Startaddr in Vflyspi_adc::start " << startaddr;
		log(Logger::DEBUG0) << "Endaddr in Vflyspi_adc::start " << endaddr;
		log(Logger::DEBUG0) << "single_mode:" << single_mode_bit;
		log(Logger::DEBUG0) << "trigger_enable:" << trigger_enabled_bit;
		log(Logger::DEBUG0) << "trigger_channel:" << trigger_channel_bit;
		log(Logger::DEBUG0) << "triggerenabled:" << triggerenabled_bit;
		log(Logger::DEBUG0) << "triggered:" << triggered_bit;
		log << Logger::flush;	
	}

/*
* 	writes startadress endadress singlemode triggerenabled trigger_channel
*	thisby configuring operation mode of adc board
*/
    void Vflyspi_adc::set_addr(sp6data *buf, uint32_t startaddr, uint32_t endaddr) {
        if(endaddr < startaddr){
            throw std::runtime_error("end address must be larger than start address");
        }
        //consider max packet size for USB (see e.g. http://msdn.microsoft.com/en-us/library/windows/hardware/ff538112)
        //if > 4Msamples, there will be a libusb error with errno=11
        if(endaddr - startaddr > 4 * 1024 * 1024 - 1){
            Logger& log = Logger::instance();
            log(Logger::WARNING) << "read in chunks < 4 Msamples to avoid libusb timeouts";
        }
        //maximum number of double words (dw) in 256MB memory: 256MB * 8bit / 32bit = 32Mdw
        //TP: these numbers are for Spikey board and may differ for ADC board
        if(endaddr > 256 * 1024 * 1024 / 4 - 1){
            throw std::runtime_error("memory address out of range");
        }
		writeBuf(buf, startadr_msb,(startaddr>>24)&0xff);
		writeBuf(buf, startadr_2,(startaddr>>16)&0xff);
		writeBuf(buf, startadr_1,(startaddr>>8)&0xff);
		writeBuf(buf, startadr_lsb,(startaddr>>0)&0xff);
		writeBuf(buf, endadr_msb,(endaddr>>24)&0xff);
		writeBuf(buf, endadr_2,(endaddr>>16)&0xff);
		writeBuf(buf, endadr_1,(endaddr>>8)&0xff);
		writeBuf(buf, endadr_lsb,(endaddr>>0)&0xff);
    }

	void Vflyspi_adc::setup_controller(uint32_t startaddr, uint32_t endaddr, bool single_mode,
	                                   bool trigger_enabled, bool trigger_channel,
	                                   bool activate_compression)
	{
		enable_compression = activate_compression;

		// write configuration
		sp6data *buf=writeBlock(0,18);
		set_addr(buf, startaddr, endaddr);
		// set enable and single shot bit, set start and stop bit to zero
		writeBuf(buf, startstop,
		         static_cast<sp6data>((trigger_channel << 4) | (trigger_enabled << 3) |
		                              (single_mode << 2) | 0x00));
		writeBuf(buf, compression, static_cast<sp6data>(enable_compression ? 0xffff : 0));

		doWB();
	}
	
//*** trigger functions
	
//switch on single triggered recording
	void Vflyspi_adc::set_single_trigger(){
	//check the state
		sp6data state = read(startstop);
		bool trigger_channel = (state >> 4) & 0x1;
	//reset
		sp6data *buf=writeBlock(0,4);
		bool trigger_enabled = 0;
		bool single_mode = 0;
		writeBuf(buf, startstop,static_cast<sp6data>((trigger_channel<<4) | (trigger_enabled<<3) | (single_mode<<2) | 0x00 ));
	//set	
		trigger_enabled = 1;
		single_mode = 1;
		writeBuf(buf, startstop,static_cast<sp6data>((trigger_channel<<4) | (trigger_enabled<<3) | (single_mode<<2) | 0x00 ));
	//tell ocp
		doWB();
	}

//switch off triggered recording
	void Vflyspi_adc::set_off_trigger(){
	//check the state
		sp6data state = read(startstop);
                bool trigger_channel = (state >> 4) & 0x1;
	//reset
                sp6data *buf=writeBlock(0,2);
                bool trigger_enabled = 0;
                bool single_mode = 0;
                writeBuf(buf, startstop,static_cast<sp6data>((trigger_channel<<4) | (trigger_enabled<<3) | (single_mode<<2) | 0x00 ));
	//tell ocp
		doWB();
	}


//trigger recording manually
	void Vflyspi_adc::manual_trigger() {
		sp6data *buf=writeBlock(0,4);
	//tell to start recording
		writeBuf(buf, startstop,start_val);
	//tell to end recording	
		writeBuf(buf, startstop,0);
	//tell ocp
		doWB();
	}


	uint32_t Vflyspi_adc::get_version()
	{
		// commit id is 1 bit + 7 hex numbers = 1bit + 3.5 byte
		// this results in the mask 0x1fffffff
		// and required size uint32
		uint32_t value = read(commitid&0x1fffffff);
		return value;
	}
	
