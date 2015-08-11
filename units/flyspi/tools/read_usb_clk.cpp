
using namespace std;

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
#include "Vspiconfrom.h"
#include "Vspigyro.h"
#include "Vspiwireless.h"

int main(){
	//select loggin level 
	Logger::instance("main",Logger::INFO,"");

	std::vector<Vmoduleusb*> boards;

	while (1)
	{
		try
		{
			//create the top of the tree
			Vmoduleusb* io = new Vmoduleusb(usbcomm::note);
			if(io->open(0x04b4,0x1003)){
				//cout<<"Open failed"<<endl;
				break;
			}
			else
				cout<<"Board #" << io->getSerial() << " opened"<<endl;

			boards.push_back(io);

			//create sp6 class tree
			Vusbmaster usb(io);
			//usbmaster knows three clients, must be in this order!!!
			Vusbstatus status(&usb);
			Vmemory mem(&usb);
			Vocpfifo ocp(&usb);

			cout << "Usb clock frequency " << status.getUsbClockFrequency() << endl;
		}
		catch (...)
		{
			break;
		}
	}

	for (auto ii : boards)
	{
		//cout << "Deleting pointer to board #" << ii->getSerial() << endl;
		delete ii;
	}

	return 0;

}
