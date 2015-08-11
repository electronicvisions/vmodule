// transport
#include <gtest/gtest.h>
#include "vmodule.h"
#include "Vmemory.h"
#include "Vusbmaster.h"
#include "Vocpfifo.h"
#include "Vmux_board.h"
#include "Vmodule_adc.h"


#ifdef WITH_FLANSCH
#include "Vmodulesim.h"
#else
#include "Vmoduleusb.h"
#endif

using namespace std;

TEST(LogADCStatus, LogAdcControllerConfiguration)
{
	/*
	 * Communication stack
	 */

	//select loggin level
	Logger::instance("vmodule.test.LogADCStatus",Logger::INFO,"");

	//create the top of the tree
#ifdef WITH_FLANSCH
	Vmodulesim io(50023,"vtitan.kip.uni-heidelberg.de");
#else
	Vmoduleusb io(usbcomm::note,0x04b4,0x1003);
#endif

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	//ocpfifo clients
	// spikey stuff
	Vmux_board mux_board(&ocp);
	Vflyspi_adc adc(&ocp);//,FASTADC__BASEADR);

	adc.log_status();
}


