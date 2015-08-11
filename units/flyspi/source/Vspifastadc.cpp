#include "Vspifastadc.h"
#include "Vocpfifo.h"

Vspifastadc::Vspifastadc(Vmodule<sp6adr,sp6data> *parent):Vspimodule(parent,Vocpfifo::OCP_FASTADC)
{
}
//write only, upper 3 data bits are located in address field
void Vspifastadc::write(ubyte adr, uint data)
{
	Vspimodule::write((adr&0xf)<<3 | ((data>>8)&0x7),(ubyte)(data&0xff));
}
