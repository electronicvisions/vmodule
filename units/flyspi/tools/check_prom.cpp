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
	Logger::instance("main", Logger::INFO,"");

	//create the top of the tree
	Vmoduleusb io(usbcomm::note);

	if(io.open(0x04b4,0x1003)){
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

	cout << hex << confrom.get_device_id() << endl;
	cout << hex << (unsigned int)confrom.read(Vspiconfrom::cmd_rdid) << endl;
	cout << hex << (unsigned int)confrom.read(Vspiconfrom::cmd_rdsr) << endl;

	return 0;

}
