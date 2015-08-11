/*
	vmodule tree header for usb communication
	Johannes Schemmel 2011
*/

#include "vmodule.h"
#include "vmsp6.h"
#include "logger.h"
#include "usbcom.h"


//Vmoduleusb uses usbcom to send and receive the Vmodule buffer

class Vmoduleusb:public Vmodule<sp6adr,sp6data> {
	protected:
		class usbcomm *usb;
	public:
	//constructor only creates usbcomm class and buffers
		Vmoduleusb(uint loglevel);
		Vmoduleusb(uint loglevel, uint vendor, uint device);
		Vmoduleusb(uint loglevel, uint vendor, uint device, uint serial);
		Vmoduleusb(uint loglevel, uint vendor, uint device, std::string serial);
		virtual ~Vmoduleusb();
	//open device
		int open(uint vendor, uint device);
		int open(uint vendor, uint device, uint serial);
		int open(uint vendor, uint device, std::string serial);
	//transfer functions overloaded to connect hierarchy to io stream
		void initTransfer(uint id,Vrequest<sp6adr> &req);

		void doTransfer(uint id);

		std::string getSerial();
};

