// transport
#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"
#include "Vusbstatus.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
using namespace boost::accumulators;

#ifdef WITH_FLANSCH
#include "Vmodulesim.h"
#else
#include "Vmoduleusb.h"
#endif

using namespace std;

float convert_to_voltage(unsigned int raw)
{
	return static_cast<float>(raw*(-0.000356387)+1.17011);
}

int main(int argc, char* argv[]) {
	/*
	 * Communication stack
	 */

	//select loggin level
	Logger::instance("main", argc > 1 ? Logger::DEBUG1 : Logger::INFO, "");
	Logger& log = Logger::instance();

	//create the top of the tree
#ifdef WITH_FLANSCH
	Vmodulesim io(50023,"vtitan.kip.uni-heidelberg.de");
#else
	Vmoduleusb io(usbcomm::note,0x04b4,0x1003);
#endif

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	//ocpfifo clients
	// spikey stuff
	Vmux_board mux_board(&ocp,3);
	Vflyspi_adc adc(&ocp);//,FASTADC__BASEADR);

	log(Logger::INFO) << "Serial number:" << io.getSerial();
	log(Logger::INFO) << "Git revision of ADC bitfile: " << hex << adc.get_version();

	/*
	 * ADC configuration and readout
	 */
	// configure board
	mux_board.enable_power();
	mux_board.set_Mux(Vmux_board::OUTAMP_2);
	//mux_board.set_Mux(Vmux_board::MUX_GND);
	//ocp.write(0x8000,(1<<24));
	std::cout << "read: " << hex << ocp.read(0x8000) << std::endl; 
	std::cout << "read: " << hex << ocp.read(0x8000) << std::endl; 
	std::cout << "read: " << hex << ocp.read(0x8000) << std::endl; 
	std::cout << "read: " << hex << ocp.read(0x8000) << std::endl; 

	unsigned int startaddr = 0;
	unsigned int adc_num_samples = 0;
	float sample_time_us = 1000.0;

	adc_num_samples = int(float(sample_time_us * 500) / 10.3) + 1;
	log(Logger::DEBUG0) << "Num samples: " << adc_num_samples;
	unsigned int endaddr = startaddr + adc_num_samples;

	// write configuration to adc
	adc.configure(0);
	// set adc into ready state
	adc.setup_controller(startaddr,endaddr,0 /* single mode */,0 /* trigger enable */,0 /* trigger channel */);
	adc.manual_trigger();

	// read data back from memory
	Vbufuint_p data = mem.readBlock(startaddr+0x08000000,adc_num_samples);

	std::vector<float> voltages;
	std::vector<float> times;

	for (unsigned int i=0; i<adc_num_samples*2; i=i+2)
	{
		times.push_back(i*0.0103);
		times.push_back((i+1)*0.0103);
		unsigned int value = int(data[i/2]);
		//voltages.push_back(convert_to_voltage((value>>16)&0xfff));
		//voltages.push_back(convert_to_voltage((value)&0xfff));
		voltages.push_back((value>>16)&0xfff);
		voltages.push_back((value)&0xfff);
	}

	accumulator_set<double, stats<tag::mean, tag::variance > > acc;
	for_each(voltages.begin(), voltages.end(), bind<void>(ref(acc), placeholders::_1));

    cout << mean(acc) << endl;
	cout << sqrt(variance(acc)) << endl;

	// write results to a file
	ofstream myfile;
	myfile.open ("output.txt");
	for (unsigned int i=0; i<adc_num_samples*2; i++)
	{
		myfile << times[i]<<","<<voltages[i] << "\n";
	}
	myfile.close();

}


