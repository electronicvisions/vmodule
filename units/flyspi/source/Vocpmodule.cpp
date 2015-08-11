#include "Vocpmodule.h"

void Vocpmodule::processAdr(Vrequest<sp6adr> &a){
	a.adr += baseadr;
}

void Vocpmodule::setBaseAdr(uint adr){
	baseadr = adr;
}

uint Vocpmodule::read(uint adr)
{
	Vrequest<sp6adr> r(adr,0,1);
	initTransfer(0,r);
	doTransfer();
	return (*(getInbuf()->posPtr()));
}

sp6data *Vocpmodule::writeBlock(uint /*adr*/, ts_t num)
{
	Vrequest<sp6adr> r(0,num,0);//need two fields for adr and data
	initTransfer(0,r);
	return getBuffer()->posPtr();
}

void Vocpmodule::doWB()
{
	doTransfer();
}

void Vocpmodule::write(uint adr, uint data)
{
	adr += baseadr;
	adr |= ocpwrite; //since adr goes directly into buffer, have to do all processing here
	sp6data *b=writeBlock(0,2); //adr field has no meaning, 2 entries
	*b++=(adr);
	*b=(data);
	doWB();
}
