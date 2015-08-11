#include "Vspigyro.h"

Vspigyro::Vspigyro(Vmodule<sp6adr,sp6data> *parent):Vspigyrowl(parent)
{
}

ubyte Vspigyro::read(ubyte adr)
{
	return Vspigyrowl::read_gyro(adr);
}

void Vspigyro::write(ubyte adr, ubyte data)
{
	Vspigyrowl::write_gyro(adr,data);
}

float Vspigyro::read_temperature()
{
	// reset and i2c disable
	Vspigyrowl::write_gyro(0x6a,0x1f);
	// these usleeps are actually necessary,
	// Andreas Hartel found them out experimentally
	usleep(10000);

	// wake up
	Vspigyrowl::write_gyro(0x6b,0);
	// see first usleep
	usleep(10000);

	// sample rate divider
	Vspigyrowl::write_gyro(0x19,2);
	// see first usleep
	usleep(10000);

	uint thi = Vspigyrowl::read_gyro(0x41); // temphi
	uint tlo = Vspigyrowl::read_gyro(0x42); // templo
	short int tempi = thi*256 + tlo; // 2s complement in lower 16 bit
	float temp = float(tempi)/340.0 + 36.53; // from data sheet

	return temp;
}
