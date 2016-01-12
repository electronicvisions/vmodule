/// A tool to zero the first memory bank of the AnaFB board. This is the
/// memory bank used by HALbe.
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

static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("main");

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
	return (testsize - 1) / bufsize + 1;
}

static size_t calc_words(const size_t bytes)
{
	return bytes / 4;
}

int main(int argc, char** argv)
{
	logger_default_config(log4cxx::Level::getInfo());

	std::string adc_id;

	const size_t offset = 0x08000000;    // Offset of 2nd memory bank in words
	const size_t data_size = 0x04000000; // one whole memory block -> 128MB
	const size_t block_size = 0x400000;  // write in blocks of 1MB

	const size_t block_words = calc_words(block_size);
	const size_t data_words = calc_words(data_size);
	const size_t blocks = calc_blocks(data_words, block_words);

	{
		namespace po = boost::program_options;
		po::options_description desc("This will zero the ADC memory used by HALbe");
		desc.add_options()("help", "produce help message")(
		    "adc", po::value<std::string>(&adc_id)->required(), "specify ADC board");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help")) {
			std::cout << std::endl << desc << std::endl;
			return 0;
		}
		po::notify(vm);
	}

	Vmoduleusb io(usbcomm::note, usb_vendorid, usb_deviceid, adc_id);
	LOG4CXX_INFO(logger, "Board " << adc_id << " opened");
	Vusbmaster usb(&io);
	Vusbstatus status(&usb);
	Vmemory mem(&usb);
	Vocpfifo ocp(&usb);
	Vmux_board mux_board(&ocp, mux_board_mode(adc_id));
	Vflyspi_adc adc(&ocp);

	// write configuration to adc
	adc.configure(0);

	// Do an basic transfer initialization to check that the ADC works as expected.
	LOG4CXX_INFO(logger, "Setup transfer for " << calc_words(data_size) << " words");
	adc.setup_controller(0, calc_words(data_size), 0 /* single mode */, 0 /* trigger enable */,
	                     0 /* trigger */);

	const uint32_t startaddr = adc.get_startaddr();
	const uint32_t endaddr = adc.get_endaddr();
	const uint32_t num_words = endaddr - startaddr;
	if (startaddr != 0 or endaddr != calc_words(data_size)) {
		LOG4CXX_ERROR(logger, "Invalid start or end addresses: startaddr=0x"
		                          << startaddr << " endaddr=0x" << endaddr);
		exit(-1);
	}

	LOG4CXX_INFO(logger, "Transfering " << num_words << " words in " << blocks << " block(s) of "
	                                    << block_words << " words");
	LOG4CXX_INFO(logger, "startaddr=" << std::hex << startaddr << " endaddr=" << endaddr);

	// The actualle transfer is done by direct memory access:
	// The memory is written in blocks defined by block_size, otherwise USB errors might occure.
	size_t adr = offset;
	for (size_t l = 0; l < blocks; l++) {
		size_t size = std::min(data_words - (l * block_words), block_words);
		Vbufuint_p buf = mem.writeBlock(adr, size);
		for (size_t j = 0; j < size; j++) {
			buf[j] = 0;
		}
		mem.doWB();
		adr += size;
	}
}
