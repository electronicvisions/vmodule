#pragma once
#include "vmodule.h"
#include "vmsp6.h"
#include "Vocpmodule.h"

//ocp to spi converter
class Vspimodule:public Vocpmodule {
	public:
	Vspimodule(Vmodule<sp6adr,sp6data> *parent,uint badr):Vocpmodule(parent,badr){};
	enum spiadr_m {esc=0x800,useadr=0x400,csa=0x300,spiadr=0xff};
	//HAL functions 
	ubyte read(ubyte adr, bool useruseadr=true);
	sp6data *writeBlock(uint /*adr*/, ts_t num);
	void doWB();
	void write(ubyte badr, ubyte data, bool useruseadr=true);
};

