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

//runs random memtest with size uints and loops repetitions
int64_t memtest(Vmemory &mem, ts_t testsize, uint loops);
// defined in ./vmtest.cpp


TEST(Mem,check) {
	//select loggin level
	Logger::instance("vmodule.test.Mem",Logger::INFO,"");
	Logger& log = Logger::instance();

	/*
	 * Communication stack
	 */
	//create the top of the tree
	Vmoduleusb io(usbcomm::note,0x04b4,0x1003);

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);

	log(Logger::INFO) << "Serial number:" << io.getSerial();

	while (1)
		for(int sz = 1*1024*1024; sz < 9*1024*1024; sz += 1024*1024)
			ASSERT_EQ(memtest(mem,sz,1),0);
}


