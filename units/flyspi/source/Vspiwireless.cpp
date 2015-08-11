#include "Vspiwireless.h"

Vspiwireless::Vspiwireless(Vmodule<sp6adr,sp6data> *parent):Vspigyrowl(parent)
{
}

ubyte Vspiwireless::read(ubyte adr)
{
	return Vspigyrowl::read_wl(adr);
}

void Vspiwireless::write(ubyte adr, ubyte data)
{
	Vspigyrowl::write_wl(adr,data);
}
