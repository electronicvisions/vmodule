#include "Vspidac.h"
#include <bitset>

// user-defined literal
// skip them until g++ 4.7 is available
/*
sp6data operator "" _V(float voltage)
{ return uint(voltage/1.8*4096); }

sp6data operator "" _mV(float voltage)
{ return (voltage/1000)_V; }
*/

//HAL
void Vspikeydac::enableReference() {
    sp6data *buf = writeBlock(0,4);
    *(buf++) = ocpwrite | (baseadr + 0x408);
    *(buf++) = 0;
    *(buf++) = ocpwrite | (baseadr + 0xc00);
    *buf = 0x01;
    doWB();
}

void Vspikeydac::disableReference() {
    sp6data *buf = writeBlock(0,4);
    *(buf++) = ocpwrite | (baseadr + 0x408);
    *(buf++) = 0;
    *(buf++) = ocpwrite | (baseadr + 0xc00);
    *buf = 0;
    doWB();
}

void Vspikeydac::setCurrent_uA(Channel channel, float current){
    // conversion consists of two parts
    // 1. current to voltage value (1e6 for uA and 30000 for 30kOhm)
    // 2. convert voltage to DAC value 
    std::bitset<14> data((1.8 - (current / 1000000. * 30000.0)) / 2.5 * 16384.);

    if (channel != IREFDAC)
        throw std::logic_error("Can't set current to channels other than IREFDAC in Vspikeydac");
	uint channelTmp = 6;

	//std::cout << "Writing " << current << " uA to channel " << channelTmp << std::endl;

    sp6data *buf = writeBlock(0,4);
    *(buf++) = ocpwrite | (baseadr + 0x403);
    *(buf++) = ((channelTmp&0xf) << 4) | ((data>>10).to_ulong()&0xf);
    *(buf++) = ocpwrite | (baseadr + 0xc00) | ((data>>2).to_ulong() & 0xff);
    *buf = (data.to_ulong()&0x3)<<6;
    doWB();
}

void Vspikeydac::setVoltage(Channel channel, float voltage){
    std::bitset<14> data(voltage / 2.5 * 16384.);

	uint channelTmp;
	if(boardVersion == 1){ //spikey board version 1
		switch(channel){
			case VCASDAC: channelTmp = 7; break;
			case VM: channelTmp = 5; break;
			case VREST: channelTmp = 3; break;
			case VSTART: channelTmp = 1; break;
			default: throw std::logic_error("Can't set voltage to channel IREFDAC in Vspikeydac");
		}
	}
	else if(boardVersion == 2){ //spikey board version 2
		switch(channel){
			case VCASDAC: channelTmp = 1; break;
			case VM: channelTmp = 3; break;
			case VREST: channelTmp = 5; break;
			case VSTART: channelTmp = 7; break;
			default: throw std::logic_error("Can't set voltage to channel IREFDAC in Vspikeydac");
		}
	}
	else{
		std::string msg = "Board version " + std::to_string(boardVersion) + " not supported";
		throw std::logic_error(msg);
	}

	//std::cout << "Writing " << voltage << " volts to channel " << channelTmp << std::endl;

    sp6data *buf = writeBlock(0,4);
    *(buf++) = ocpwrite | (baseadr + 0x403);
    *(buf++) = ((channelTmp&0xf) << 4) | ((data>>10).to_ulong()&0xf);
    *(buf++) = ocpwrite | (baseadr + 0xc00) | ((data>>2).to_ulong() & 0xff);
    *buf = (data.to_ulong()&0x3)<<6;
    doWB();
}
