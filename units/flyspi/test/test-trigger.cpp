
using namespace std;

#include <gtest/gtest.h>
#include <string>
#include <list>
#include <fstream>

#include "logger.h"

#include "vmodule.h"
#include "vmsp6.h"
#include "Vmoduleusb.h"
#include "Vusbmaster.h"
#include "Vusbstatus.h"
#include "Vmemory.h"
#include "Vocpfifo.h"
#include "Vocpmodule.h"
#include "Vmodule_adc.h"


TEST(Vmodule_adc, trigger)
{
	//select loggin level 
	Logger::instance("vmodule.test.Vmodule_adc",Logger::DEBUG0,"");


	//create the top of the tree
	Vmoduleusb io(usbcomm::note);
	if(io.open(0x04b4,0x1003)){
		cout<<"Open failed"<<endl;
	}
	else
		cout<<"Board #" << io.getSerial() << " opened"<<endl;

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	Vflyspi_adc adc(&ocp);

	cout << "Usb clock frequency " << status.getUsbClockFrequency() << endl;

	adc.set_single_trigger();
	adc.log_status();
	Vflyspi_adc::Status st = adc.get_status();
	EXPECT_EQ(st.triggered_bit,0);

	adc.setup_controller(0,100,0,0,0);
	sleep(2);
	adc.setup_controller(0,100,1,1,0);
	adc.log_status();
	st = adc.get_status();
	EXPECT_EQ(st.triggered_bit,0);

}
