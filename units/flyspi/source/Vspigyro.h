#pragma once
#include "vmsp6.h"
#include "vmodule.h"
#include "Vspigyrowl.h"

class Vspigyro:public Vspigyrowl {
	public:
		Vspigyro(Vmodule<sp6adr,sp6data> *parent);
		ubyte read(ubyte adr);
		void write(ubyte adr, ubyte data);

		float read_temperature();
};

