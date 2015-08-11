#pragma once
#include "vmsp6.h"
#include "vmodule.h"
#include "Vspimodule.h"

class Vspifastadc:public Vspimodule {
	public:
	Vspifastadc(Vmodule<sp6adr,sp6data> *parent);
	//write only, upper 3 data bits are located in address field
	void write(ubyte adr, uint data);
};


