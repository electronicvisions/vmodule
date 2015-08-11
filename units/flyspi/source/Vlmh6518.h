#pragma once

#include "vmsp6.h"
#include "Vocpmodule.h"

class Vlmh6518 : protected Vocpmodule
{
	public:
		Vlmh6518(Vmodule<sp6adr,sp6data> *parent,uint badr);

		enum preamp {LG=0,HG=1};
		enum filter {FULL=0, M20=1, M100=2,M200=3,M350=4,M650=5,M750=6};
/*
		void set_preamp(preamp);
		void set_attenuator(int dB);
		void set_filter(filter);
*/
		void configure(bool,filter,preamp,int);
};

