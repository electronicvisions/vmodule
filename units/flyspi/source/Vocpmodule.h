#pragma once
#include "vmsp6.h"
#include "vmodule.h"

//ocpmodule is base class for all ocp clients (zero copy version)
class Vocpmodule:public Vmodule<sp6adr,sp6data> {
	protected:
	uint baseadr;
	public:
	enum ocpcmd {ocpwrite=0x80000000};
	Vocpmodule(Vmodule<sp6adr,sp6data> *parent,uint badr):Vmodule<sp6adr,sp6data>(parent),baseadr(badr){};
	//add base address
	virtual void processAdr(Vrequest<sp6adr> &a);
	virtual void setBaseAdr(uint adr);
	//HAL functions 
	uint read(uint adr);
	sp6data *writeBlock(uint /*adr*/, ts_t num);
	void doWB();
	void write(uint adr, uint data);
};
