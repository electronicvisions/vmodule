/*
	header file for vmodule classes for spikey on sp6board
	Andreas Gruebl 2012
*/

#pragma once
#include "vmodule.h"
#include <array>

// new electronic visions slow-control base class for sp6board

using namespace std;

#include <cstring>
#include <list>
#include "logger.h"

#include "ocpfifoadr.h"
#include "Vocpmodule.h"
#include "Vocpfifo.h"
#include "Vspimodule.h"
#include "Vmodule_adc.h"


// Spikey controller related:
class Vspikeyctrl:public Vocpmodule {
	public:
	Vspikeyctrl(Vmodule<sp6adr,sp6data> *parent):Vocpmodule(parent,Vocpfifo::OCP_SPY_CTRL){};
	uint read(uint adr){return Vocpmodule::read(adr&0xfff); };
	void write(uint adr, uint data){Vocpmodule::write(adr&0xfff,data);};
};

class Vspikeypbmem:public Vocpmodule {
	public:
	Vspikeypbmem(Vmodule<sp6adr,sp6data> *parent, uint myBoardVersion):Vocpmodule(parent,Vocpfifo::OCP_SPY_PBMEM){
		switch(myBoardVersion){
			case 1: setBaseAdr(Vocpfifo::OCP_SPY_PBMEM);break;
			case 2: setBaseAdr(Vocpfifo::OCP_SPY_PBMEM_V2);break;
		}
	};
	uint read(uint adr){return Vocpmodule::read(adr&0xfff); };
	void write(uint adr, uint data){Vocpmodule::write(adr&0xfff,data);};
};

class Vspikeydelcfg:public Vocpmodule {
	public:
	Vspikeydelcfg(Vmodule<sp6adr,sp6data> *parent, uint myBoardVersion):Vocpmodule(parent,Vocpfifo::OCP_SPY_DELCFG){
		switch(myBoardVersion){
			case 1: setBaseAdr(Vocpfifo::OCP_SPY_DELCFG);break;
			case 2: setBaseAdr(Vocpfifo::OCP_SPY_DELCFG_V2);break;
		}
	};
	uint read(uint adr){return Vocpmodule::read(adr&0xfff); };
	void write(uint adr, uint data){Vocpmodule::write(adr&0xfff,data);};
};

/** Access to ADC2365 on Spikey board. */
class Vspikeyslowadc:protected Vocpmodule {
	private:
		uint boardVersion;
    public:
	Vspikeyslowadc(Vmodule<sp6adr,sp6data> *parent, uint myBoardVersion):Vocpmodule(parent, Vocpfifo::OCP_SPY_SLOWADC){
		boardVersion = myBoardVersion;
	};

	//HAL
	float readVoltage(uint channel=0);
};

/** Access to ADS6125 on USB board. */
class Vspikeyfastadc:public Vflyspi_adc {
    public:
    Vspikeyfastadc(Vmodule<sp6adr,sp6data> *parent):Vflyspi_adc(parent){};

    void setup_controller(uint32_t startaddr, uint32_t endaddr);
};
