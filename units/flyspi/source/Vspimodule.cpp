#include "Vspimodule.h"

ubyte Vspimodule::read(ubyte adr, bool useruseadr)
{
	Vrequest<sp6adr> r(adr|(useruseadr?useadr:0)|esc,0,1); //baseadr will be added in inittransfer
	initTransfer(0,r);
	doTransfer();
	return (ubyte)(*(getInbuf()->posPtr()));
}

sp6data *Vspimodule::writeBlock(uint /*adr*/, ts_t num)
{
	Vrequest<sp6adr> r(0,num,0);//need two fields for adr and data
	initTransfer(0,r);
	return getBuffer()->posPtr();
}

void Vspimodule::doWB()
{
	doTransfer();
}

void Vspimodule::write(ubyte badr, ubyte data, bool useruseadr)
{
	uint adr=badr;
	adr += baseadr; //since adr goes directly into buffer, have to do all processing here
	adr |= ocpwrite | (useruseadr?useadr:0) |esc;
	sp6data *b=writeBlock(0,2); //adr field has no meaning, 2 entries
	*b++=(adr);
	*b=(data);
	doWB();
}
