#pragma once

#include "vmodule.h"
#include "vmsp6.h"
#include "ocpfifoadr.h"

//ocpfifo (zerocopy) is a zero copy version of ocpfifo
//ocpfifo accepts only in=0,out=n or in=1,out=0
class Vocpfifo:public Vmodule<sp6adr,sp6data>,public Ocpfifoadr {
public:
//	enum ocpadr {OCP_CONFROM=0,OCP_FASTADC=0x1000,OCP_GYROWL=0x2000,OCP_ADCCTRL=0x3000};
	Vocpfifo(Vmodule<sp6adr,sp6data> *parent);
	virtual void initTransfer(uint id, Vrequest<sp6adr> &a);

	virtual void doTransfer(uint id=0);
	//HAL functions (for debugging only, direct ocp read/write, little endian data
	uint read(uint adr);
	sp6data *writeBlock(uint /*adr*/, ts_t num);
	void doWB();
	void write(uint adr, uint data);
};

