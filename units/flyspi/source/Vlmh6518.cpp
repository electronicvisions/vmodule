#include "Vlmh6518.h"
#include <bitset>
#include <exception>
#include <iostream>

using namespace std;

Vlmh6518::Vlmh6518(Vmodule<sp6adr,sp6data> *parent,uint badr):Vocpmodule(parent,badr)
{
}

void Vlmh6518::configure(bool aux, filter fil,preamp pre,int dB)
{
	// check attenuation
	if (dB < 0) dB *= -1;
	if (dB%2 > 0) throw "Attenuation needs to be an even number <= 20. You can pass it over with either sign, it will always be interpreted as a negative number.";
	if (dB > 20) throw "Attenuation needs to be an even number <= 20. You can pass it over with either sign, it will always be interpreted as a negative number.";
	dB /= 2;

	cout << "Attenuation/2: " << dB << endl;
	cout << "Filter: " << fil << endl;

	bitset<16> spi_data(0);
	spi_data |= dB;
	spi_data.set(4,pre);
	spi_data |= fil<<6;
	spi_data |= aux<<10;

	cout << spi_data << endl;

	sp6data *buf = writeBlock(0,4);
	*(buf++) = ocpwrite | (baseadr + 0x000);
	*(buf++) = 0x00;
	*(buf++) = ocpwrite | (baseadr + 0xc00) | ((spi_data.to_ulong()>>8)&0xff);
	*buf = (spi_data.to_ulong()&0xff);
	doWB();
}
