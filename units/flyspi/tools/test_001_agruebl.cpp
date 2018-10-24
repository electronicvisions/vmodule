// Some test script for Spikey 5
// by Andreas Grübl
//
// transport
#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vusbstatus.h"
#include "Vspikey.h"
#include "Vspidac.h"

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

int main(int argc, char* /*argv*/[]) {
	/*
	 * Communication stack
	 */

	//select loggin level
	Logger::instance("main", argc > 1 ? Logger::INFO : Logger::INFO, "");
	Logger& log = Logger::instance();

	//create the top of the tree
#ifdef WITH_FLANSCH
	Vmodulesim io(50023,"vtitan.kip.uni-heidelberg.de");
#else
	Vmoduleusb io(usbcomm::note,0x04b4,0x1003);
#endif

	//create sp6 class tree
	Vusbmaster     usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus     status(&usb);
	Vmemory        mem(&usb);
	Vocpfifo       ocp(&usb);
	//ocpfifo clients
	Vmux_board     mux_board(&ocp, 2); //TODO: TP: Spikey board v2 is mode 3!
	Vspikeyslowadc adc(&ocp, 2);
	Vspikeyctrl    spyctrl(&ocp);
	Vspikeydac     spydac(&ocp, 2);

	log(Logger::INFO) << "Serial number:" << io.getSerial();

	uint sg_scpower_pos  = 0;
	uint sg_scrst_pos    = 4;
	uint sg_sccim_pos    = 8;
	uint sg_scdirectout  = 9;
	uint sg_scplr_pos    = 12;
	uint sg_scvrest_pos  = 16;
	uint sg_scvm_pos     = 20;
	uint sg_scibtest_pos = 24;
	
	uint sg_scmode = 0;

	uint dout = true  << sg_scpower_pos |
	            true  << sg_scrst_pos |
	            false << sg_sccim_pos |
	            false << sg_scdirectout |
	            false << sg_scplr_pos |
	            false << sg_scvrest_pos |
	            false << sg_scvm_pos |
	            false << sg_scibtest_pos;
	std::cout << "dout: " << std::hex << dout << std::endl;
	spyctrl.write(sg_scmode, dout);
	
	/*
	 * ADC readout
	 */

	mux_board.enable_power();
	
	spydac.enableReference();
	spydac.setCurrent_uA(Vspikeydac::IREFDAC, 25);
	spydac.setVoltage(Vspikeydac::VCASDAC, 1.6);
	spydac.setVoltage(Vspikeydac::VM, 0.4);
	spydac.setVoltage(Vspikeydac::VREST, 0.8);
	spydac.setVoltage(Vspikeydac::VSTART, 1.2);

	float voltage = adc.readVoltage(1);
	
	std::cout << "returned value: " << voltage << std::endl;
}
