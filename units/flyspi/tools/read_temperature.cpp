using namespace std;

#include <string>
#include <list>
#include <iostream>
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
	Logger::instance("main",Logger::INFO,"");
	logger_default_config(log4cxx::Level::getInfo());
	//create the top of the tree
	Vmoduleusb io(usbcomm::note);

	std::string board_id;

	{
		namespace po = boost::program_options;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("board_id", po::value<std::string>(&board_id)->required(), "specify board")
		;

		po::positional_options_description pos;

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

	}

	if(io.open(0x04b4, 0x1003, board_id)){
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

	status.setStatus(0);
	cout << status.getStatus() << endl;
	cout << hex << confrom.get_device_id() << dec << endl;

	cout << "Whoami: " << (unsigned int)gyro.read_gyro(0x75) << endl;

	float temp = gyro.read_temperature();
	cout << "Temperature (deg. Celsius): " << temp << endl;
}

