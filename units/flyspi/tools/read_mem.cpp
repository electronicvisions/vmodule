#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <boost/program_options.hpp>

#include "Vmemory.h"
#include "Vmodule_adc.h"
#include "Vmoduleusb.h"
#include "Vmux_board.h"
#include "Vocpfifo.h"
#include "Vspiconfrom.h"
#include "Vspifastadc.h"
#include "Vspigyro.h"
#include "Vspiwireless.h"
#include "Vusbmaster.h"
#include "Vusbstatus.h"
#include "vmodule.h"
#include "vmsp6.h"

#include "logging_ctrl.h"

static const unsigned int usb_vendorid = 0x04b4;
static const unsigned int usb_deviceid = 0x1003;

static unsigned mux_board_mode(std::string serial)
{
	if (serial.size() > 1 && serial[0] == 'B')
		return 2;
	else
		return 0;
}

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

int main(int argc, char** argv){
	//select loggin level 
	logger_default_config(log4cxx::Level::getInfo());

	std::string adc_id;
	std::string filename;
	size_t offset = 0x08000000; // 128M * 4 -> second memory bank
	size_t block_size = 2048;
	size_t data_size = 4096;
	bool simple_mode = false;
	bool halbe_mode = false;
	bool statistic_mode = false;

	{
		namespace po = boost::program_options;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("adc", po::value<std::string>(&adc_id)->required(), "specify ADC board")
			("data_size", po::value<size_t>(&data_size), "Amount of data to read (kB), default 4096kB")
			("block_size", po::value<size_t>(&block_size), "Read data in blocks of this size(kB), default 2048kB")
			("simple_mode", po::bool_switch(&simple_mode), "Run forever and collect statistics.")
			("halbe_mode ", po::bool_switch(&halbe_mode), "Run forever and collect statistics.")
			("statistic_mode", po::bool_switch(&statistic_mode), "Run forever and collect statistics.")
		;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv)
						.options(desc)
						.run()
				, vm);

		if (vm.count("help")) {
			std::cout << std::endl << desc << std::endl;
			return 0;
		}
		po::notify(vm);
	}

	if (simple_mode)
	{
		//create the top of the tree
		Vmoduleusb io(usbcomm::note);

		if(io.open(usb_vendorid, usb_deviceid, adc_id)){
			std::cout << "Open failed" << std::endl;
			return -1;
		}
		else {
			std::cout << "Board " << adc_id << " opened" << std::endl;
		}

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
		std::cout << "Transfering " << data_size << "kB in "
			      << calc_blocks(data_size * 1024, block_size * 1024)
				  << " block(s) of "
				  << calc_words(block_size) << " words." << std::endl;
		size_t errorcnt = memtest(mem, calc_words(data_size), offset, calc_words(block_size));
		std::cout << "Transmission errors: " << errorcnt << std::endl;
		if (errorcnt > 0)
			return -1;
	}
    if (halbe_mode)
	{
		Vmoduleusb io(usbcomm::note, usb_vendorid, usb_deviceid, adc_id);
		std::cout << "Board " << adc_id << " opened" << std::endl;
		Vusbmaster usb(&io);
		Vusbstatus status(&usb);
		Vmemory mem(&usb);
		Vocpfifo ocp(&usb);
		Vmux_board mux_board(&ocp, mux_board_mode(adc_id));
		Vflyspi_adc adc(&ocp);

		// write configuration to adc
		adc.configure(0);
		adc.setup_controller(0, calc_words(data_size),
				0 /* single mode */, 0 /* trigger enable */, 0 /* trigger */);

		const uint32_t startaddr = adc.get_startaddr();
		const uint32_t endaddr   = adc.get_endaddr();
		const uint32_t num_words = endaddr - startaddr;
		if (startaddr != 0 or endaddr != calc_words(data_size))
		{
			std::cout << "Invalid start or end addresses: startaddr="
				      << startaddr << " endaddr=" << endaddr;
			exit(-1);
		}
		std::cout << "Transfering " << num_words << " words in "
			      << calc_blocks(data_size * 1024, block_size * 1024)
				  << " block(s) of "
				  << num_words << " words." << std::endl;
		size_t errorcnt = memtest(mem, num_words, offset, calc_words(block_size));
		std::cout << "Transmission errors: " << errorcnt << std::endl;
		if (errorcnt > 0)
			return -1;
	}
}
