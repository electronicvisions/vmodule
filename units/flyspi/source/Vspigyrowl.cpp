#include "Vspigyrowl.h"

Vspigyrowl::Vspigyrowl(Vmodule<sp6adr,sp6data> *parent):Vspimodule(parent,Vocpfifo::OCP_GYROWL)
{
}

//HAL functions
ubyte Vspigyrowl::read_gyro(ubyte adr)
{
	return Vspimodule::read((adr&0x7f)+gyroadr+0x80);
} //gyro needs 0x80 as read cmd, 0x40 is burst cmd
void Vspigyrowl::write_gyro(ubyte adr, ubyte data)
{
	Vspimodule::write((adr&0x7f)+gyroadr,data);
}
ubyte Vspigyrowl::read_wl(ubyte adr)
{
	return Vspimodule::read((adr&0x7f)+wladr);
}
void Vspigyrowl::write_wl(ubyte adr, ubyte data)
{
	Vspimodule::write((adr&0x7f)+wladr+0x80,data);
} //wl needs 0x80 as write cmd
