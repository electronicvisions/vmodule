#include "Vusbstatus.h"
#include "Vrequest.hpp"

//HAL functions
uint Vusbstatus::getStatus()
{
	Vrequest<sp6adr> r(0,0,1);
	initTransfer(0,r);
	doTransfer();
	return (*(getInbuf()->begin()));//read one entry from buffer
}

void Vusbstatus::setStatus(uint s)
{
	Vrequest<sp6adr> r(0,1,0);
	initTransfer(0,r);
	getBuffer()->push_back((s));
	doTransfer();
}

double Vusbstatus::getUsbClockFrequency()
{
	return 1e6 * ( (getStatus()>>8)&0xff );
}

