/*
       * libusb test for bulk transfer

	log levels: 0 nothing
							1 error
							2 warning
							3 note
							4 data
*/
#include <gtest/gtest.h>
      #include <stdlib.h>
      #include <stdio.h>
      #include <sys/types.h>
			#include <cstring>
			#include <iostream>

			#include <vmodule.h>
#include "usbcom.h"
      
      #include <libusb.h>
      using namespace std;
			typedef unsigned int uint;
			
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
			
	int readhex(const char * mcsfile,unsigned char *buf,size_t bufsize);

namespace {
 
	void ocpw(class usbcomm &usb,uint addr, uint data=0){
		uint a=addr|0x80000000;uint d=data;
		if(usb.raw(usbcomm::WRITEOCP,a,d)){printf("Raw failed\n");
		}else printf("Raw WRITEOCP: %02x %02x\n", a, d);				
	}
	
	uint ocpf(class usbcomm &usb){
		uint d;uint a=0;
		if(usb.raw(usbcomm::READOCPFIFO,a,d)){printf("Raw failed\n");
		}else printf("Raw READOCPFIFO: %x %x\n", a, d);				
		if(a&0x80000000)return ocpf(usb); else		return d;
	}

	uint ocpr(class usbcomm &usb,uint addr,bool nofiforead=false){
		uint a=addr	;uint d;
		if(usb.raw(usbcomm::READOCP,a,d)){printf("Raw failed\n");
		}else 
			if(a&0x80000000 && nofiforead==false)d=ocpf(usb);//fifo empty after read, so read fifo again
			else printf("Raw READOCP: %02x %02x\n", a, d);				
		return d;
	}
	
	void wirelessinit(class usbcomm &usb){
		ocpw(usb,0x2d80|MODE_OVERRIDE,0x19); //mode_override full reset of device and osc always on
		ocpw(usb,0x2d80|PWR_CTRL,0x0); //disable pmu
		ocpw(usb,0x2d80|CHANNEL,48); //select radio channel
		ocpw(usb,0x2d80|TX_CFG,0x2f); //tx_cfg set 8dr mode
		ocpw(usb,0x2d80|RX_CTRL,0x00); //rx_ctrl no irq
		ocpw(usb,0x2d80|RX_CFG,0x8a); //rx_cfg
		ocpw(usb,0x2d80|XACT_CFG,0x13); //xact_cfg no ack, end state rx
		ocpw(usb,0x2d80|FRAMING_CFG,0xee); //framing_cfg 
		ocpw(usb,0x2d80|DATA32_THOLD,0x03); //data32_thold
		ocpw(usb,0x2d80|DATA64_THOLD,0x07); //data64_thold
		ocpw(usb,0x2d80|CRC_SEED_LSB,0x14); //crc_seed_lsb
		ocpw(usb,0x2d80|CRC_SEED_MSB,0x14); //crc_seed_msb
		ocpw(usb,0x2d80|TX_OFFSET_LSB,0x55); //tx_offset_lsb
		ocpw(usb,0x2d80|TX_OFFSET_MSB,0x05); //tx_offset_msb
		ocpw(usb,0x2d80|RX_OVERRIDE,0x08); //rx_override
		ocpw(usb,0x2d80|XTAL_CFG,0x08); //xtal_cfg
		ocpw(usb,0x2d80|AUTO_CAL_TIME,0x3c); //datasheets value
		ocpw(usb,0x2d80|AUTO_CAL_OFFSET,0x14); //datasheets value
	}

}

TEST(USBSpi, IDontKnowWhatItDoes)
	{
		//printf("%ld",sizeof(int));
		usbcomm myusb(0),myusb2(0);

		if(myusb.open_first(0x04b4,0x1003)){
			printf("open failed\n");
			ASSERT_EQ(0, myusb.status); return;;
		}else printf("opened.\n");

		if(myusb2.open_first(0x04b4,0x1003)){
			printf("open second boardfailed\n");

		}else {
			printf("opened second board.\n");
			myusb2.align();	
			myusb2.selfrefresh(3); //disable all memories
		}
		myusb.align();	
		myusb.selfrefresh(0); //enable all memories

		uint result;
		uint data1=0xaabbeecc,data2=0xdeaddead;

		if(myusb.write(0x100,data1)){
			printf("write failed\n");ASSERT_EQ(0, myusb.status); return;;
		}else printf("Write.");
		if(myusb.write(0x101,data2)){
			printf("write failed\n");ASSERT_EQ(0, myusb.status); return;;
		}else printf("Write.");

		if(myusb.read(0x100,result)){
			printf("read failed\n");ASSERT_EQ(0, myusb.status); return;;
		}else {
			printf("Read1: %02x ", result);
			ASSERT_EQ(result,data1);
		}
		if(myusb.read(0x101,result)){
			printf("read failed\n");ASSERT_EQ(0, myusb.status); return;;
		}else {
		    printf("Read2: %02x\n", result);
			ASSERT_EQ(result,data2);
		}


		myusb.selfrefresh(0x0); //disable only mcb0 for adc access

		//test raw 

		unsigned int addr=0x80000400;
		if(myusb.raw(usbcomm::WRITEOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("Raw WRITESPI: %02x %02x\n", addr, result);				

		for(int m=0;m<2;m++){
			if(m==0)	result = 0x80;//enable loopback
			else 			result = 0x00; // no loopback

			myusb.raw(usbcomm::WRITESTATUS,addr,result);
			if(myusb.raw(usbcomm::READSTATUS,addr,result)){
				printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("Raw: %02x %02x\n", addr, result);				

			for(int i=0;i<10;i++){
				addr=i;
				result=1000-i;
				if(myusb.raw(usbcomm::READOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else printf("Raw READOCP: %02x %02x\n", addr, result);				
			}
		}			
		//now read back config rom id code
		for(int i=0;i<3;i++)ocpr(myusb,0xc05);
		ocpr(myusb,0x49f);ocpr(myusb,0x0);ocpr(myusb,0x0);ocpr(myusb,0x800);

		ocpw(myusb,0x403,0x0);
		ocpw(myusb,0x400,0x0);

		addr=0x0;
		int numrw=32;
		for(int i=0;i<numrw;i++){
			if(i==numrw-1)addr=0x800; //release spi cs
			if(myusb.raw(usbcomm::READOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("%02x ", result);
			if(!((i+1)%16))printf("\n");			
		} 

		//now read gyro
		ocpw(myusb,0x2c6a,0x1f);//reset and i2c disable
		//need to wait after reset
		usleep(20000);
		ocpw(myusb,0x2c6b,0);//wake up
		ocpw(myusb,0x2c19,2);//sample rate divider

		ocpr(myusb,0x2cea);//ifconfig
		ocpr(myusb,0x2ceb);//pwr_mgt
		ocpr(myusb,0x2cf5);//whoami

  	uint thi=ocpr(myusb,0x2cc1);//temphi
		uint tlo=ocpr(myusb,0x2cc2);//templo
		short int tempi=thi*256+tlo;//2s complement in lower 16 bit
		
		float temp=tempi/325.0+30; //from data sheet
	  cout<<"Temperature: "<<tempi<<" "<<temp<<endl;

  	thi=ocpr(myusb,0x2cc1);//temphi
		tlo=ocpr(myusb,0x2cc2);//templo
		tempi=thi*256+tlo;//2s complement in lower 16 bit
		
		temp=tempi/325.0+30; //from data sheet
	  cout<<"Temperature: "<<tempi<<" "<<temp<<endl;

//again with burst
		unsigned int *bbuf=myusb.getBuffer(10000);//just a large one
		ASSERT_TRUE(bbuf) << "Alloc buffer failed.";
//		bbuf[0]=__builtin_bswap32(0x80002c6a);bbuf[1]=__builtin_bswap32(0x1f);
		bbuf[0]=__builtin_bswap32(0x80002d00);bbuf[1]=__builtin_bswap32(0x1f);
		bbuf[2]=__builtin_bswap32(0x80002d00);bbuf[3]=__builtin_bswap32(0x0);
		bbuf[4]=__builtin_bswap32(0x80002c19);bbuf[5]=__builtin_bswap32(0x2);
		bbuf[6]=__builtin_bswap32(0x80002c6b);bbuf[7]=__builtin_bswap32(0x0);
		bbuf[8]=__builtin_bswap32(0x80002c6b);bbuf[9]=__builtin_bswap32(0x0);
		myusb.burstwriteocp(8);

  	thi=ocpr(myusb,0x2cc1);//temphi
		tlo=ocpr(myusb,0x2cc2);//templo
		tempi=thi*256+tlo;//2s complement in lower 16 bit
		
		temp=tempi/325.0+30; //from data sheet

	  cout<<"Temperature: "<<tempi<<" "<<temp<<endl;


//END TEST	

		//test adc
		ocpw(myusb,0x3008,0x08); //start clock
		ocpw(myusb,0x3008,0xe8); //release pdn and reset for adc, no start signal yet
		ocpw(myusb,0x1c00,0x10); //software reset
		ocpw(myusb,0x1c01,0x4); //enable lvds interface, disable clock
//		ocpw(myusb,0x1c00|(0xe<<3)|2,0x81); //7mA, 100Ohm
		ocpw(myusb,0x1c00|(0xe<<3),0xc); //lowest power lvds is 0xc
		ocpw(myusb,0x1c00|(0x4<<3)|0,0x0); //clock edge
		ocpw(myusb,0x1c00|(0xb<<3)|1,0x54); //custom pattern 0x555 ->datablatt wrong!!!
		ocpw(myusb,0x1c00|(0xc<<3)|0,0x05); //custom pattern 0x555 ->datablatt wrong!!!
		ocpw(myusb,0x1c00|(0xa<<3)|4,0x00); //enable ramp test pattern with 0x80, custom with 0xa0
		
		//ocp burst write to set start- end- and cfg 
		bbuf=myusb.getBuffer(10000);//just a large one
//		unsigned int *bbuf=myusb.getBuffer(10000);//just a large one
		ASSERT_TRUE(bbuf) << "Alloc buffer failed.";
		bbuf[0]=__builtin_bswap32(0x80003000);bbuf[1]=__builtin_bswap32(0);
		bbuf[2]=__builtin_bswap32(0x80003001);bbuf[3]=__builtin_bswap32(0);
		bbuf[4]=__builtin_bswap32(0x80003002);bbuf[5]=__builtin_bswap32(0);
		bbuf[6]=__builtin_bswap32(0x80003003);bbuf[7]=__builtin_bswap32(0);
		bbuf[8]=__builtin_bswap32(0x80003004);bbuf[9]=__builtin_bswap32(0);
		bbuf[10]=__builtin_bswap32(0x80003005);bbuf[11]=__builtin_bswap32(0);
		bbuf[12]=__builtin_bswap32(0x80003006);bbuf[13]=__builtin_bswap32(1);
		bbuf[14]=__builtin_bswap32(0x80003007);bbuf[15]=__builtin_bswap32(0);//end address
		bbuf[16]=__builtin_bswap32(0x80003009);bbuf[17]=__builtin_bswap32(0x1);//start
 	  bbuf[18]=__builtin_bswap32(0x80003009);bbuf[19]=__builtin_bswap32(0x0);//start off
		myusb.burstwriteocp(20);
		
		addr=0x3000;
		numrw=32;
		for(int i=0;i<11;i++){
			ocpr(myusb,addr+i);
		} 
		
		addr=0x08000000;
		numrw=32;
		for(int i=0;i<numrw;i++){
			if(myusb.read(addr+i,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("%08x ", result);
			if(!((i+1)%8))printf("\n");			
		} 
		ocpw(myusb,0x3008,0x0); //switch off adc

/*		
		//check cypress wireless
		usbcomm &tx=myusb,&rx=myusb2;

//		for(int x=0;x<0x3f;x++)ocpr(myusb,0x2d00+x);
		
		//config myusb2 as sender, myusb as receiver

		wirelessinit(tx);
		wirelessinit(rx);								
		
		//start receive on myusb
		cout<<"receive"<<endl;
		ocpw(rx,0x2d80|RX_CTRL,0x80); //rx go
//		ocpw(rx,0x2d80|XACT_CFG,0x13); //end state rx
			
		ocpr(rx,0x2d00|TX_IRQ_STATUS);	
		ocpr(rx,0x2d00|RX_IRQ_STATUS);	
		ocpr(rx,0x2d00|RX_STATUS); 
		
		//send something
		cout<<"transmit"<<endl;
		ocpr(tx,0x2d00|TX_CTRL);	
		ocpr(tx,0x2d00|TX_IRQ_STATUS);	

//		ocpw(tx,0x2d80|RX_CTRL,0x80); //rx go (see website)
		ocpw(tx,0x2d80|XACT_CFG,0x27); //end state idle	
		ocpw(tx,0x2d80|TX_LENGTH,8); //0 to 16 bytes
		ocpw(tx,0x2d80|TX_CTRL,0x40); //clears transmit buffer	
		ocpr(tx,0x2d00|TX_IRQ_STATUS);	
		ocpw(tx,0x2d80|TX_BUFFER,0x33); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xaa); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0x00); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xff); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xde); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xad); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xde); //test data
		ocpw(tx,0x2d80|TX_BUFFER,0xad); //test data
		ocpw(tx,0x2d80|TX_CTRL,0x80); //tx go transmits
		
		ocpr(tx,0x2d00|TX_IRQ_STATUS);	
		ocpr(tx,0x2d00|TX_IRQ_STATUS);	
		cout<<"receive"<<endl;
		ocpr(rx,0x2d00|RX_IRQ_STATUS);	
		ocpr(rx,0x2d00|RX_STATUS); 		
		ocpr(rx,0x2d00|RX_COUNT); 		
		ocpr(rx,0x2d00|RX_LENGTH); 
		ocpr(rx,0x2d00|RX_IRQ_STATUS);	
		ocpr(rx,0x2d00|RSSI);	
		
		ocpw(rx,0x2d80|RX_IRQ_STATUS,0x80); //write to transfer read data to buffer
		ocpr(rx,0x2d00|RX_IRQ_STATUS); 
		for(int i=0;i<8;i++)	ocpr(rx,0x2d00|RX_BUFFER);
		ocpr(rx,0x2d00|RX_IRQ_STATUS); 
		ocpr(rx,0x2d00|RX_IRQ_STATUS); 
		*/
		myusb.selfrefresh(0x03); //disable mcbs

}



	//no known bugs at the moment


