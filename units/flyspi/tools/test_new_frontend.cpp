	// transport
#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"
#include "Vlmh6518.h"
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

int main () {
	/*
	 * Communication stack
	 */

	//select loggin level
	Logger::instance("main",Logger::INFO,"");
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
	Vmux_board mux_board(&ocp);
	Vflyspi_adc adc(&ocp);//,FASTADC__BASEADR);
	Vlmh6518 lmh(&ocp,0x9000);

	log(Logger::INFO) << "Serial number:" << io.getSerial();

	/*
	 * ADC configuration and readout
	 */
	// configure board
	mux_board.enable_power();
	//mux_board.set_Mux(Vmux_board::MUX_GND);
	mux_board.set_Mux(Vmux_board::OUTAMP_0);
	//ocp.write(0x8000,(1<<27));
	//ocp.write(0x8000,0);

	int filter;
	cin >> filter;
	Vlmh6518::filter filt;
	if (filter==0)
		filt=Vlmh6518::FULL;
	else if (filter==20)
		filt=Vlmh6518::M20;
	else if (filter==100)
		filt=Vlmh6518::M100;
	else if (filter==200)
		filt=Vlmh6518::M200;
	else if (filter==350)
		filt=Vlmh6518::M350;
	else if (filter==650)
		filt=Vlmh6518::M650;
	else if (filter==750)
		filt=Vlmh6518::M750;
	else  filt=Vlmh6518::FULL;

	int damping;
	cin >> damping;

	bool aux;
	cin >> aux;

	lmh.configure(aux,filt,Vlmh6518::LG,damping);


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
	myfile.open ("/tmp/output.txt");
	for (unsigned int i=0; i<adc_num_samples*2; i++)
	{
		myfile << times[i]<<","<<voltages[i] << "\n";
	}
	myfile.close();
}
