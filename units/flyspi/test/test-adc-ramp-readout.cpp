// transport
#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"
#include "Vusbstatus.h"
#include "Vmoduleusb.h"

#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;


TEST(ADC,Ramp) {
	/*
	 * Communication stack
	 */

	//select loggin level
	Logger::instance("vmodule.test.ADC",Logger::INFO,"");
	Logger& log = Logger::instance();

	//create the top of the tree
	Vmoduleusb io(usbcomm::note,0x04b4,0x1003);

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	//ocpfifo clients
	// spikey stuff
	Vmux_board mux_board(&ocp);
	Vflyspi_adc adc(&ocp);//,FASTADC__BASEADR);

	log(Logger::INFO) << "Serial number:" << io.getSerial();

	/*
	 * ADC configuration and readout
	 */
	// configure board
	ocp.write(0x8000,(1<<27));

	unsigned int startaddr = 0;
	unsigned int adc_num_samples = 0;
	float sample_time_us = 1000.0;

	adc_num_samples = int(float(sample_time_us * 500) / 10.3) + 1;
	log(Logger::DEBUG0) << "Num samples: " << adc_num_samples;
	unsigned int endaddr = startaddr + adc_num_samples;

	// write configuration to adc
	adc.configure(4);
	// set adc into ready state
	adc.setup_controller(startaddr,endaddr,0 /* single mode */,0 /* trigger enable */,0 /* trigger channel */);
	adc.manual_trigger();

	// read data back from memory
	Vbufuint_p data = mem.readBlock(startaddr+0x08000000,adc_num_samples);

	std::vector<unsigned int> samples;
	unsigned int last_value = (data[0]>>16)&0xfff;

	for (unsigned int i=0; i<adc_num_samples*2; i=i+2)
	{
		unsigned int value = int(data[i/2]);

		samples.push_back((value>>16)&0xfff);

		if (samples.back() == 0 && last_value>0)
			ASSERT_GE(last_value,4095);
		else
			ASSERT_GE(samples.back(),last_value);

		last_value = samples.back();

		samples.push_back((value)&0xfff);

		if (samples.back() == 0 && last_value>0)
			ASSERT_GE(last_value,4095);
		else
			ASSERT_GE(samples.back(),last_value);

		last_value = samples.back();
	}
}


