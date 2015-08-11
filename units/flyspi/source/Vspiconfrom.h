#pragma once
#include <string>
#include "vmsp6.h"
#include "vmodule.h"
#include "Vspimodule.h"
#include "Vocpfifo.h"

//spi clients
class Vspiconfrom:public Vspimodule {
	public:
	static const uint SECSIZE=0x10000; //64kB sector
	Vspiconfrom(Vmodule<sp6adr,sp6data> *parent):Vspimodule(parent,Vocpfifo::OCP_CONFROM){};
	enum regs  //config rom registers/commands
		{cmd_wren=0x06, //write enable
		 cmd_wrdi=0x04, //write disable
		 cmd_rdid=0x9f, //read identification
		 cmd_rdsr=0x05, //read status reg
		 cmd_wrsr=0x01, //write status reg
		 cmd_read=0x03, //read data bytes
		 cmd_fread=0x0b, //read data high speed
		 cmd_pp=0x02, //page program
		 cmd_se=0xd8, //sector erase
		 cmd_be=0xc7, //bulk erase
		 cmd_res=0xab}; //read electronic signature
	//HAL functions
	//program from file
	int program(const std::string &name);

	//single byte read/write
	ubyte read(ubyte adr, bool useruseadr=true );
	void write(ubyte adr, ubyte data, bool useruseadr=true);

	sp6data get_device_id ();

	//multi-byte read/write functions
	//read a block from config rom
	void read_mem(uint adr,ubyte *data,ts_t num);

	//writes a sector and reprograms it, polls for erase completion, blocks and is slow!!!
	//sector size is 64k
	void write_sector(uint startadr, ubyte *data);
};
