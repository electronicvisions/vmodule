/*
	vmodule sp6board usage example
	Johannes Schemmel 2011
*/


// new electronic visions slow-control base class

using namespace std;

#include <string>
#include <list>
#include <fstream>

#include <boost/program_options.hpp>

#include "logging_ctrl.h"

#include "vmodule.h"
#include "vmsp6.h"
#include "Vmoduleusb.h"
#include "Vusbmaster.h"
#include "Vusbstatus.h"
#include "Vmemory.h"
#include "Vocpfifo.h"
#include "Vocpmodule.h"
#include "Vspiconfrom.h"
#include "Vspigyro.h"
#include "Vspiwireless.h"

int main(int argc, char** argv){
	//select loggin level 
	logger_default_config(log4cxx::Level::getInfo());

	std::string adc_id;
	string filename;

	{
		namespace po = boost::program_options;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("adc", po::value<std::string>(&adc_id)->required(), "specify ADC board")
			("bitfile", po::value<std::string>(&filename)->required(), "bitfile")
		;

		po::positional_options_description pos;
		pos.add("bitfile", 1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv)
						.options(desc)
						.positional(pos)
						.run()
				, vm);

		if (vm.count("help")) {
			std::cout << std::endl << desc << std::endl;
			return 0;
		}
		po::notify(vm);

		// check if filename is correct
		ifstream ifile(filename);
		if (!ifile)
		{
			cerr << "File " << filename << " not found!" << endl;
			return -1;
		}
	}

	//create the top of the tree
	Vmoduleusb io(usbcomm::note);

	if(io.open(0x04b4, 0x1003, adc_id)){
		cout<<"Open failed"<<endl;exit(-1);
	}else cout<<"Board opened"<<endl;

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

	// print some status
	status.setStatus(0);
	cout << status.getStatus() << endl;
	cout << hex << confrom.get_device_id() << dec << endl;

	//some memory io
	mem.write(0,0x33333333);
	mem.write(1,0xaaaaaaaa);
	assert(mem.read(0)==0x33333333);
	assert(mem.read(1)==0xaaaaaaaa);

	//for(uint i=0;i<Vspiconfrom::SECSIZE;i++)test[i]=i;

	uint secadr=0;
	int numread=256;
	ubyte buf[numread];
	confrom.read_mem(secadr,buf,numread);
	for(int i=0;i<numread;i++){
		printf("%02x ", buf[i]);
		if(!((i+1)%16))printf("\n");
	}

	int error=confrom.program(filename);
	printf("Result of write: %d\n",error);

	return 0;

}
