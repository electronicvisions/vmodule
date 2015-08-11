/*
	vmodule tree header for simulator communication
	Andreas Hartel 2012
*/

#include "flansch_software.h"
#include "vmsp6.h"
#include "vmodule.h"
#include <string>

#ifndef VMODULESIM
#define VMODULESIM

class Vmodulesim:public Vmodule<sp6adr,sp6data> {
protected:
    flanschSoftware flansch;
public:
	Vmodulesim(int port, std::string hostname):Vmodule<sp6adr,sp6data>(), flansch(port,hostname) {
		inbuf = new Vbuffer<sp6data>{};
		outbuf = new Vbuffer<sp6data>{};
	}
	virtual ~Vmodulesim(){
		delete inbuf;
		delete outbuf;
	};
//transfer functions overloaded to connect hierarchy to io stream
	void initTransfer(uint id,Vrequest<sp6adr> &req);

	void doTransfer(uint id);
	int getSerial() { return 0;}
};

#endif
