#include "vmsp6.h"
#include "vmodule.h"

//ddr3 memory interface
class Vmemory:public Vmodule<sp6adr,sp6data>
{
	public:
	Vmemory(Vmodule<sp6adr,sp6data> *parent);
	//HAL functions
	Vbufuint_p writeBlock(uint adr, ts_t num);
	//using Vbufptr instead of plain uint*
	Vbufuint_p readBlock(uint adr, ts_t num);
	void doWB();
	uint read(uint adr);
	void write(uint adr, uint data);
};

