#include "Vocpfifo.h"

//	enum ocpadr {OCP_CONFROM=0,OCP_FASTADC=0x1000,OCP_GYROWL=0x2000,OCP_ADCCTRL=0x3000};
Vocpfifo::Vocpfifo(Vmodule<sp6adr,sp6data> *parent):Vmodule<sp6adr,sp6data>(parent)
{
}

void Vocpfifo::initTransfer(uint id, Vrequest<sp6adr> &a)
{
	requests[id]=a;
	Vrequest<sp6adr> padr; //request for parent
	if(a.in){//read 
		padr.adr=a.adr; //only used for read
		padr.in=1;
	}else	padr.out=a.out;//caller has to alloc two entries per ocp transfer
	getBuffer()->alloc(a.out);//alloc fifo entries (in ocpfifo-buffer format)
	Vmodule<sp6adr,sp6data>::initTransfer(id,padr);
}

void Vocpfifo::doTransfer(uint id)
{
	//not used anymore
	if(requests[id].in){ //read
		Vmodule<sp6adr,sp6data>::doTransfer(id);
	}else{ //write burst		
		Vmodule<sp6adr,sp6data>::doTransfer(id);
	}
}

//HAL functions (for debugging only, direct ocp read/write, little endian data
uint Vocpfifo::read(uint adr)
{
	Vrequest<sp6adr> r(adr,0,1);
	initTransfer(0,r);
	doTransfer();
	return (*(getInbuf()->posPtr()));
}

sp6data* Vocpfifo::writeBlock(uint /*adr*/, ts_t num)
{
	Vrequest<sp6adr> r(0,num,0);//adr has no meaning for ocpfifo
	initTransfer(0,r);
	return getBuffer()->posPtr();
}

void Vocpfifo::doWB()
{
	doTransfer();
}

void Vocpfifo::write(uint adr, uint data)
{
	adr |= 0x80000000; //since adr goes directly into buffer, have to do all processing here
	sp6data *b=writeBlock(0,2); //adr field has no meaning, 2 entries
	*b++=(adr);
	*b=(data);
	doWB();
}
