#include "Vmemory.h"

//ddr3 memory interface

Vmemory::Vmemory (Vmodule<sp6adr,sp6data> *parent):Vmodule<sp6adr,sp6data>(parent)
{
}

Vbufuint_p Vmemory::writeBlock(uint adr, ts_t num)
{
	Vrequest<sp6adr> r(adr,num,0);
	initTransfer(0,r);
	return Vbufuint_p(getBuffer());
}

//using Vbufptr instead of plain uint*
Vbufuint_p Vmemory::readBlock(uint adr, ts_t num)
{
	Vrequest<sp6adr> r(adr,0,num);
	initTransfer(0,r);
	doTransfer();
	return Vbufuint_p(getInbuf());
}

void Vmemory::doWB()
{
	doTransfer();
}

uint Vmemory::read(uint adr)
{
	Vbufuint_p b = readBlock(adr,1);
	uint ret = (uint)*b;
	return ret;
}

void Vmemory::write(uint adr, uint data)
{
	Vbufuint_p b=writeBlock(adr,1);
	*b=data;
	doWB();
}
