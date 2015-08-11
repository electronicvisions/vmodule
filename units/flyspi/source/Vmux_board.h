#pragma once
#include "vmsp6.h"
#include <bitset>
#include "Vocpmodule.h"


class Vmux_board : protected Vocpmodule {

	public:
		Vmux_board(Vmodule<sp6adr,sp6data> *parent, uint my_mode=0):Vocpmodule(parent, 0) {
			shadow_copy_Xctrl = 0;
			//shadow_copy_IOconfig = 0;
			mode = my_mode;
            if(mode == 0) reset_mask = std::bitset<32>(0x0ff3ffff);      // old spikey board as analog front end
            else if(mode == 1) reset_mask = std::bitset<32>(0xfffffccc); // old spikey board with spikey
            else if(mode == 2) reset_mask = std::bitset<32>(0xffff30ff); // new spikey board as analog front end
            else if(mode == 3) reset_mask = std::bitset<32>(0xfffffccc); // new spikey board with spikey
            else throw std::logic_error("Vmux_board(): invalid mode");
		};

		enum bits {
			POWER = 0,
			RESET = 4,
			CI_MODE = 8,
			PLL_RESET = 12
		};

		// This enum contains 32-bit xctrl masks
		// All bits, except the bits relevant for mux control
		// are set to zero. And only the bits relevant for the
		// current mux input are set to one.
		// The mux-relevant bits are
		// 18/19 for MUX1
		// 28/29 for MUX2
		// 30/31 for MUX3
		// The patterns are used in set_Mux()
		enum mux_input {
			OUTAMP_0,
			OUTAMP_1,
			OUTAMP_2,
			OUTAMP_3,
			OUTAMP_4,
			OUTAMP_5,
			OUTAMP_6,
			OUTAMP_7,
			MUX_GND
		};

		//HAL
		/*
		void set_Xctrl(std::bitset<5>);
		void unset_Xctrl(std::bitset<5>);
		void set_Xctrl_reg(unsigned int reg);
		void set_Xctrl_reg(std::bitset<32> const&);
		*/

		void set_Mux(mux_input in);
		void set_Mux(unsigned int in);

		void enable_power();

		using Vocpmodule::read;

		void write_xctrl_reg(std::bitset<32> const& reg);
	private:
		//mode = 0 for adc board
		//mode = 1 for spikey
		uint mode;
		static const unsigned int xctrl_reg_adr_adc = 0x8000;
		static const unsigned int xctrl_reg_adr_spikey = 0x8009;
        std::bitset<32> reset_mask;
		std::bitset<32> shadow_copy_Xctrl;

		//bool check_bit_range(std::bitset<5> bit);

		using Vocpmodule::write;
};

