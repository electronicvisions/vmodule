/*
	vmodule tree header for usb communication
	Johannes Schemmel 2011
*/

using namespace std;

#include "Vmoduleusb.h"

//Vmoduleusb uses usbcom to send and receive the Vmodule buffer

Vmoduleusb::Vmoduleusb(uint loglevel):Vmodule<sp6adr,sp6data>() {
	inbuf=new Vbuffer<sp6data>;
	outbuf=new Vbuffer<sp6data>;
	usb=new usbcomm(loglevel);
}

Vmoduleusb::Vmoduleusb(uint loglevel, uint vendor, uint device):Vmodule<sp6adr,sp6data>() {
	Logger& log = Logger::instance();
	inbuf=new Vbuffer<sp6data>;
	outbuf=new Vbuffer<sp6data>;
	usb=new usbcomm(loglevel);
	// RAII :)
	if (open(vendor,device)) {
	    throw std::runtime_error("Could not connect to USB board!");
	}
	else log(Logger::INFO) << "Connected to USB board.";
};

Vmoduleusb::Vmoduleusb(uint loglevel, uint vendor, uint device, uint serial):Vmodule<sp6adr,sp6data>() {
	Logger& log = Logger::instance();
	inbuf=new Vbuffer<sp6data>;
	outbuf=new Vbuffer<sp6data>;
	usb=new usbcomm(loglevel);
	// RAII :)
	if (open(vendor,device,serial)) {
	    throw std::runtime_error("Could not connect to USB board!");
	}
	else log(Logger::INFO) << "Connected to USB board.";
};

Vmoduleusb::Vmoduleusb(uint loglevel, uint vendor, uint device, std::string serial) :
	Vmodule<sp6adr,sp6data>()
{
	Logger& log = Logger::instance();
	inbuf=new Vbuffer<sp6data>;
	outbuf=new Vbuffer<sp6data>;
	usb=new usbcomm(loglevel);
	// RAII :)
	if (open(vendor,device,serial)) {
	    throw std::runtime_error("Could not connect to USB board!");
	}
	else log(Logger::INFO) << "Connected to USB board.";
};


Vmoduleusb::~Vmoduleusb() {
	delete inbuf;
	delete outbuf;
	delete usb;
}
//open device 
int Vmoduleusb::open(uint vendor, uint device) {
	if(!(usb->open_first(vendor,device))) {
		usb->align(); //initialize communication with FPGA
	}
	return usb->status; //!=0 if open failed
}

int Vmoduleusb::open(uint vendor, uint device, uint serial) {
	if(!(usb->open(vendor,device,serial))) {
		usb->align(); //initialize communication with FPGA
	}
	return usb->status; //!=0 if open failed
}

int Vmoduleusb::open(uint vendor, uint device, std::string serial) {
	if(!(usb->open(vendor,device,serial))) {
		usb->align(); //initialize communication with FPGA
	}
	return usb->status; //!=0 if open failed
}

//transfer functions overloaded to connect hierarchy to io stream
void Vmoduleusb::initTransfer(uint id,Vrequest<sp6adr> &req) {
	requests[id]=req;
	getBuffer()->alloc(requests[id].out);
}

void Vmoduleusb::doTransfer(uint id) {
	ubyte *out,*in;
	out=(ubyte*)getBuffer()->data();
	getInbuf()->clear();
	getInbuf()->alloc(requests[id].in);
	in=(ubyte*)getInbuf()->data();
	usb->bulktrans(out,in,requests[id].out*sizeof(sp6data),requests[id].in*sizeof(sp6data));
	getBuffer()->clear(); //buffer is sent
}

std::string Vmoduleusb::getSerial()
{
	return usb->getSerial();
}

