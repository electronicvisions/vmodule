#pragma once
#include "ocpfifoadr.h"
#include "Vocpmodule.h"
#include "Vocpfifo.h"


/** Access to DAC8168 */
class Vspikeydac : protected Vocpmodule {

    public:
        Vspikeydac(Vmodule<sp6adr,sp6data> *parent, uint myBoardVersion):Vocpmodule(parent,Vocpfifo::OCP_SPY_SPIDAC){
			boardVersion = myBoardVersion;
			switch(boardVersion){
				case 1: setBaseAdr(Vocpfifo::OCP_SPY_SPIDAC);break;
				case 2: setBaseAdr(Vocpfifo::OCP_SPY_SPIDAC_V2);break;
			}
		}

        enum Channel {VCASDAC, IREFDAC, VM, VREST, VSTART};

        //HAL
        void enableReference();
        void disableReference();
        void setCurrent_uA(Channel channel, float current);
        void setVoltage(Channel channel, float voltage);

	private:
		uint boardVersion;
};
