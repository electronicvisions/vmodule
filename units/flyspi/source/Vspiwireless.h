#pragma once
#include "Vspigyrowl.h"

class Vspiwireless:public Vspigyrowl {
	public:
			//cypress wireless register addresses
			enum CWL_REG { CHANNEL  					=0x00,
											TX_LENGTH  				=0x01,
											TX_CTRL  					=0x02,
											TX_CFG 						=0x03,
											TX_IRQ_STATUS			=0x04,
											RX_CTRL						=0x05,
											RX_CFG 						=0x06,
											RX_IRQ_STATUS			=0x07,
											RX_STATUS					=0x08,
											RX_COUNT 					=0x09,
											RX_LENGTH  				=0x0A,
											PWR_CTRL 					=0x0B,
											XTAL_CTRL					=0x0C,
											IO_CFG 						=0x0D,
											GPIO_CTRL					=0x0E,
											XACT_CFG 					=0x0F,
											FRAMING_CFG  			=0x10,
											DATA32_THOLD 			=0x11,
											DATA64_THOLD 			=0x12,
											RSSI 							=0x13,
											EOP_CTRL 					=0x14,
											CRC_SEED_LSB 			=0x15,
											CRC_SEED_MSB 			=0x16,
											TX_CRC_LSB 				=0x17,
											TX_CRC_MSB 				=0x18,
											RX_CRC_LSB 				=0x19,
											RX_CRC_MSB 				=0x1A,
											TX_OFFSET_LSB  		=0x1B,
											TX_OFFSET_MSB  		=0x1C,
											MODE_OVERRIDE  		=0x1D,
											RX_OVERRIDE  			=0x1E,
											TX_OVERRIDE				=0x1F,
											XTAL_CFG 					=0x26,
											CLK_OVERRIDE 			=0x27,
											CLK_EN 						=0x28,
											RX_ABORT 					=0x29,
											AUTO_CAL_TIME  		=0x32,
											AUTO_CAL_OFFSET  	=0x35,
											ANALOG_CTRL  			=0x39,
											TX_BUFFER  				=0x20,
											RX_BUFFER  				=0x21,
											SOP_CODE 					=0x22,
											DATA_CODE  				=0x23,
											PREAMBLE 					=0x24,
											MFG_ID 						=0x25};
	Vspiwireless(Vmodule<sp6adr,sp6data> *parent);
	ubyte read(ubyte adr);
	void write(ubyte adr, ubyte data);
};

