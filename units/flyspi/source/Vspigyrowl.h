#pragma once
#include "vmodule.h"
#include "vmsp6.h"
#include "Vspimodule.h"
#include "Vocpfifo.h"

class Vspigyrowl:public Vspimodule {
	public:
	enum adr_m {gyroadr=0,wladr=0x100}; //ocpspi base addresses for diffrent spi cs
	Vspigyrowl(Vmodule<sp6adr,sp6data> *parent);
	//HAL functions
	ubyte read_gyro(ubyte adr); //gyro needs 0x80 as read cmd, 0x40 is burst cmd
	void write_gyro(ubyte adr, ubyte data);
	ubyte read_wl(ubyte adr);
	void write_wl(ubyte adr, ubyte data); //wl needs 0x80 as write cmd
};

