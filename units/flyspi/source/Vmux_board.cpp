#include "Vmux_board.h"
#include <bitset>

#include <log4cxx/logger.h>

static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("vmodule.Vmux_board");


    //HAL
    void Vmux_board::write_xctrl_reg(std::bitset<32> const& reg)
	{
		/*
#pragma GCC diagnostic push
	    // ECM: uuh, warnings off!
#pragma GCC diagnostic ignored "-Wparentheses"
        uint32_t scmode_reg = 0xffff & reg.to_ulong() | reg[21]<<24;
        uint32_t anamux_reg = reg[18] | reg[19]<<1 | reg[28]<<4 | reg[29]<<5 | reg[30]<<8 | reg[31]<<9;
#pragma GCC diagnostic pop
		*/
		uint32_t xctrl_reg = reg.to_ulong();

		uint xctrl_reg_adr = 0;
		if(mode == 0 || mode == 2) {
			xctrl_reg_adr = xctrl_reg_adr_adc;
		} else if(mode == 1 || mode == 3) {
			xctrl_reg_adr = xctrl_reg_adr_spikey;
		}
		sp6data *buf = writeBlock(0,2);
		*(buf++) = ocpwrite | xctrl_reg_adr;
		*(buf++) = xctrl_reg;
		doWB();
    }

/*
    bool Vmux_board::check_bit_range(std::bitset<5> bit)
	{
        switch (bit.to_ulong()) {
            case 0:
                return 1;
            case 4:
                return 1;
            case 8:
                return 1;
            case 12:
                return 1;
            case 18:
                return 1;
            case 19:
                return 1;
            case 21:
                return 1;
            case 28:
                return 1;
            case 29:
                return 1;
            case 30:
                return 1;
            case 31:
                return 1;
            default:
                return 0;
        }
    }
*/

    /*
     * XCTRL methods
     */
/*
    void Vmux_board::set_Xctrl(std::bitset<5> bit)
	{
        if (!check_bit_range(bit)) throw length_error("bit not implemented in Vmux_board");

        // enable single xctrl pin given by number in argument
        shadow_copy_Xctrl.set(bit.to_ulong());
        write_xctrl_reg( shadow_copy_Xctrl );
    }

    void Vmux_board::unset_Xctrl(std::bitset<5> bit)
	{
        if (!check_bit_range(bit)) throw length_error("bit not implemented in Vmux_board");

        // unset single xctrl pin given by number in argument
        shadow_copy_Xctrl.set(bit.to_ulong(), false);
        write_xctrl_reg( shadow_copy_Xctrl );
    }

    void Vmux_board::set_Xctrl_reg(unsigned int reg)
	{
		set_Xctrl_reg(static_cast<std::bitset<32>>(reg));
	}

    void Vmux_board::set_Xctrl_reg(std::bitset<32> const& reg)
	{
        shadow_copy_Xctrl = reg;
        write_xctrl_reg( shadow_copy_Xctrl );
    }
*/

    void Vmux_board::set_Mux(mux_input in)
	{
		uint mux_adr = 0;
		if(mode == 0) {
			LOG4CXX_DEBUG(logger, "set mux in mode 0");
			switch(in) {   // old spikey board as analog front end
				case OUTAMP_0: mux_adr = 1<<18; break;
				case OUTAMP_1: mux_adr = 1<<19; break;
				case OUTAMP_2: mux_adr = 1<<18 | 1<<19; break;
				case OUTAMP_3: mux_adr = 1<<28; break;
				case OUTAMP_4: mux_adr = 1<<29; break;
				case OUTAMP_5: mux_adr = 1<<28 | 1<<29; break;
				case OUTAMP_6: mux_adr = 1<<30; break;
				case OUTAMP_7: mux_adr = 1<<31; break;
				case MUX_GND:  mux_adr = 1<<30 | 1<<31; break;
			}

		} else if(mode == 1) {
			LOG4CXX_DEBUG(logger, "set mux in mode 1");
			switch(in) {   // old spikey board with spikey
				case OUTAMP_0: mux_adr = 1; break;
				case OUTAMP_1: mux_adr = 2; break;
				case OUTAMP_2: mux_adr = 3; break;
				case OUTAMP_3: mux_adr = 1<<4; break;
				case OUTAMP_4: mux_adr = 2<<4; break;
				case OUTAMP_5: mux_adr = 3<<4; break;
				case OUTAMP_6: mux_adr = 1<<8; break;
				case OUTAMP_7: mux_adr = 2<<8; break;
				case MUX_GND:  mux_adr = 3<<8; break;
			}

		} else if(mode == 2) {
			LOG4CXX_DEBUG(logger, "set mux in mode 2");
			switch(in) {   // new spikey board as analog front end
				case OUTAMP_0: mux_adr = 2 << 14; break;
				case OUTAMP_1: mux_adr = 1 << 14; break;
				case OUTAMP_2: mux_adr = 3 << 10; break;
				case OUTAMP_3: mux_adr = 2 << 10; break;
				case OUTAMP_4: mux_adr = 1 << 10; break;
				case OUTAMP_5: mux_adr = 3 <<  8; break;
				case OUTAMP_6: mux_adr = 2 <<  8; break;
				case OUTAMP_7: mux_adr = 1 <<  8; break;
				case MUX_GND:  mux_adr = 3 << 14; break;
			}

		} else if(mode == 3) {
			LOG4CXX_DEBUG(logger, "set mux in mode 3");
			switch(in) {   // new spikey board with spikey
				case OUTAMP_0: mux_adr = 2 << 0; break;
				case OUTAMP_1: mux_adr = 1 << 0; break;
				case OUTAMP_2: mux_adr = 3 << 4; break;
				case OUTAMP_3: mux_adr = 2 << 4; break;
				case OUTAMP_4: mux_adr = 1 << 4; break;
				case OUTAMP_5: mux_adr = 3 << 8; break;
				case OUTAMP_6: mux_adr = 2 << 8; break;
				case OUTAMP_7: mux_adr = 1 << 8; break;
				case MUX_GND:  mux_adr = 3 << 0; break;
			}
		}

        shadow_copy_Xctrl &= reset_mask;
        shadow_copy_Xctrl |= std::bitset<32>(mux_adr);

        // better in terms of code reuse would be:
        // set_Xtctrl_reg(shadow_copy_Xctrl);
        // but this would assign a variable a reference to itself, ugly

        write_xctrl_reg(shadow_copy_Xctrl );
    }

    void Vmux_board::set_Mux(unsigned int in)
	{
        shadow_copy_Xctrl &= reset_mask;
        shadow_copy_Xctrl |= std::bitset<32>(in);

        // better in terms of code reuse would be:
        // set_Xtctrl_reg(shadow_copy_Xctrl);
        // but this would assign a variable a reference to itself, ugly

        write_xctrl_reg(shadow_copy_Xctrl );
    }

    void Vmux_board::enable_power()
	{
		if(mode==0 || mode==1)
			shadow_copy_Xctrl.set(27);
		else if (mode==2 || mode==3)
			shadow_copy_Xctrl.set(24);
		else
			throw std::logic_error("Vmux_board(): Spikey power currently must be controlled by sc_sctrl!");
		write_xctrl_reg(shadow_copy_Xctrl);
	}
