
using namespace std;

#include <string>
#include <list>
#include <iostream>
#include <fstream>

#include "logging_ctrl.h"

#include "vmodule.h"
#include "vmsp6.h"
#ifdef WITH_FLANSCH
#include "Vmodulesim.h"
#else
#include "Vmoduleusb.h"
#endif
#include "Vusbmaster.h"
#include "Vusbstatus.h"
#include "Vmemory.h"
#include "Vocpfifo.h"
#include "Vocpmodule.h"
#include "Vspiconfrom.h"
#include "Vspigyro.h"
#include "Vspiwireless.h"


static size_t calc_blocks(const size_t testsize, const size_t bufsize)
{
	return (testsize-1)/bufsize + 1;
}

static size_t calc_words(const size_t kilobytes)
{
	return kilobytes * 256;
}

//runs random memtest with size uints and loops repetitions
int64_t memtest(Vmemory &mem,
		const size_t testsize, const uint offset, const size_t bufsize=0x200000)
{
	int64_t errors=0;
	size_t blocks = calc_blocks(testsize, bufsize);

	int seed=time(NULL);

	srand(seed);
	size_t adr=offset;
	for(size_t l=0;l<blocks;l++){
		size_t size = std::min(testsize - (l * bufsize), bufsize);
		Vbufuint_p buf=mem.writeBlock(adr, size);
		for (size_t j=0; j < size; j++) {
			buf[j]=rand();
		}
		mem.doWB();
		adr+=size;
		if (adr > 0x4000000 && adr < 0x8000000) // 256MB to 512MB are not there
			throw std::runtime_error("Invalid address"); // FIXME later
	}

	adr=offset;
	srand(seed);
	for(size_t l=0;l<blocks;l++){
		size_t size = std::min(testsize - (l * bufsize), bufsize);
		Vbufuint_p buf=mem.readBlock(adr,size); //buffer is locked while buf exists
		for(size_t j=0;j<size;j++){
			uint test=rand();
			if(test!=buf[j]){
				errors++;
			}
		}
		adr+=size;
		if (adr > 0x4000000 && adr < 0x8000000)
			throw std::runtime_error("Invalid address"); // FIXME later
	}
	return errors;
}


int main(){
	//select loggin level 
	logger_default_config(log4cxx::Level::getTrace());

	static const unsigned int usb_vendorid = 0x04b4;
	static const unsigned int usb_deviceid = 0x1003;
	int errors = 0;

#ifdef WITH_FLANSCH
	//create the top of the tree
	Vmodulesim io(50024,"vmimas.kip.uni-heidelberg.de");
#else
	Vmoduleusb io(usbcomm::note);
	if(io.open(usb_vendorid, usb_deviceid)){
		std::cout << "Open failed" << std::endl;
		return -1;
	}
	else {
		std::cout << "Board " << io.getSerial() << " opened" << std::endl;
	}
#endif

	//create sp6 class tree
	Vusbmaster usb(&io);
	//usbmaster knows three clients, must be in this order!!!
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	//ocpfifo clients
	Vspiconfrom confrom(&ocp);
	Vspigyro gyro(&ocp);
	Vspiwireless wireless(&ocp);

	cout << "Usb clock frequency " << status.getUsbClockFrequency() << endl;

	size_t offset = 0x08000000; // 128M * 4 -> second memory bank

	// This memtest sends 4 MB of data to the second memory bank
	// The buffer size is 2 MB

	size_t data_size = 1;
	size_t block_size = 1;

	//size_t block_size = 2048;
	//size_t data_size = 4096;
	errors = memtest(mem,calc_words(data_size),offset,calc_words(block_size));


	return errors;
}
