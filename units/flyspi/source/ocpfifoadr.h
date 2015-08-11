#pragma once

/*
	ocpfifo address map mixin class
	Johannes Schemmel 2012
*/

class Ocpfifoadr{
public:
    enum ocpadr {OCP_CONFROM       = 0,
                 OCP_FASTADC       = 0x1000,
                 OCP_GYROWL        = 0x2000,
                 OCP_ADCCTRL       = 0x3000,
				 //Spikey specific
                 OCP_SPY_CTRL      = 0x8000,
				 OCP_SPY_SLOWADC   = 0xc000,
                 //Spikey specific (board v1)
				 OCP_SPY_PBMEM     = 0xa000,
                 OCP_SPY_DELCFG    = 0x9000,
                 OCP_SPY_SPIDAC    = 0xc000,
                 //Spikey specific (board v2)
                 OCP_SPY_PBMEM_V2  = 0x9000,
                 OCP_SPY_DELCFG_V2 = 0x8800,
                 OCP_SPY_SPIDAC_V2 = 0xa000,
                 OCP_SPY_SPIADC    = 0xc000};
};
