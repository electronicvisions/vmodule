/*
       * libusb test for bulk transfer

	log levels: 0 nothing
							1 error
							2 warning
							3 note
							4 data
*/
#include <gtest/gtest.h>
#include "usbcom.h"
      #include <stdlib.h>
      #include <stdio.h>
      #include <sys/types.h>
			#include <cstring>
			#include <iostream>
      
      #include <libusb.h>
      using namespace std;
			typedef unsigned int uint;

	int readhex(const char * mcsfile,unsigned char *buf,size_t bufsize);
			
namespace {
 
	void spi(class usbcomm &usb,uint addr, uint data=0, bool write=false){
		uint a=addr;uint d=data;
		if(write)
			if(usb.raw(usbcomm::WRITEOCP,a,d)){printf("Raw failed\n");
			}else printf("Raw WRITESPI: %02x %02x\n", a, d);				
		else
			if(usb.raw(usbcomm::READOCP,a,d)){printf("Raw failed\n");
			}else printf("Raw READSPI: %02x %02x\n", a, d);				
	}

	//burn spi rom
	int writerom(class usbcomm &myusb, const string &name){
		const uint secsize=0x10000; //64kB sectors
		unsigned char hexbuf[1024*1024*4] ;//max 32MBit flash !!!could not allocate enough memory
		int numread=readhex(name.c_str(),hexbuf,sizeof(hexbuf));
		if(numread<0)return numread;

		uint numsec=numread/secsize;
		if(numread%secsize)numsec++; //only program whole sectors
		
		unsigned char *b=hexbuf; //data to burn
		int size=128+6;
		unsigned int *bbuf=myusb.getBuffer(size);//128 bytes plus commands
		if(!bbuf){
				printf("Alloc buffer failed.\n");return -1;
		}
		
		for(uint sec=0;sec<numsec;sec++){
			//erase sector
			spi(myusb,0x80000806,0,true);
			spi(myusb,0xc05);

			spi(myusb,0x800004d8,sec,true); //erase sector
			spi(myusb,0x80000000,0x0,true);
			spi(myusb,0x80000800,0,true);

			uint j=0;
			uint result,addr=0xc05;	
			do {myusb.raw(usbcomm::READOCP,addr,result);j++;}
			while(result&1);

			for(uint wa=0;wa<secsize;wa+=128){//write page loop
				bbuf[0]=__builtin_bswap32(0x80000806);bbuf[1]=__builtin_bswap32(0);
	  	  bbuf[2]=__builtin_bswap32(0x80000402);bbuf[3]=__builtin_bswap32(sec);
				bbuf[4]=__builtin_bswap32(0x80000400+((wa>>8)&255));bbuf[5]=__builtin_bswap32(wa&255);
				for(int i=6;i<size-2;i+=2){
					bbuf[i]=__builtin_bswap32(0x80000400+ *b++);bbuf[i+1]=__builtin_bswap32(*b++);
				}
				bbuf[size-2]=__builtin_bswap32(0x80000c00+*b++);bbuf[size-1]=__builtin_bswap32(*b++);

				printf("%x ",wa);
				myusb.burstwriteocp(size);
				addr=0xc05;	
				do {myusb.raw(usbcomm::READOCP,addr,result);printf("%x ",result);}
				while(result&1);
			}
		printf("\n");
		}
		return 0;	
	}
	
}

TEST(USBConfigTestModBurst, IDontKnowWhatItDoes)
	{
		//printf("%ld",sizeof(int));
		usbcomm myusb(usbcomm::loglevel::note);
		//libusb_set_debug 	(NULL,3 	) 	;

		if(myusb.open_first(0x04b4,0x1003)){
			printf("open failed\n");
			ASSERT_EQ(0, myusb.status); return;;
		}else printf("opened.\n");

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


		myusb.selfrefresh(0x3); //disable all memories

		//test raw 

		unsigned int addr=0x80000400;
		if(myusb.raw(usbcomm::WRITEOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("Raw WRITESPI: %02x %02x\n", addr, result);				

		for(int m=0;m<2;m++){
			if(m==0)	result = 0x83;//enable loopback
			else 			result = 0x03; // no loopback

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
		for(int i=0;i<3;i++)spi(myusb,0xc05);
		spi(myusb,0x49f);spi(myusb,0x0);spi(myusb,0x0);spi(myusb,0x800);

		spi(myusb,0x80000403,0x48,true);
		spi(myusb,0x80000400,0x0,true);

		addr=0x0;
		int numrw=128;
		for(int i=0;i<numrw;i++){
			if(i==numrw-1)addr=0x800; //release spi cs
			if(myusb.raw(usbcomm::READOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("%02x ", result);
			if(!((i+1)%16))printf("\n");			
		} 

		//write spi rom
		spi(myusb,0x80000806,0,true);
		spi(myusb,0xc05);

		spi(myusb,0x800004d8,0x48,true); //erase sector
		spi(myusb,0x80000000,0x0,true);
		spi(myusb,0x80000800,0,true);

		uint j=0;
		addr=0xc05;	
		do {myusb.raw(usbcomm::READOCP,addr,result);j++;}
		while(result&1);
		printf("Number of polls: %d\n",j);

		spi(myusb,0x80000806,0,true);
		spi(myusb,0xc05);

		spi(myusb,0x80000402,0x48,true); //write data  
		spi(myusb,0x80000000,0x1,true);
		spi(myusb,0x80000000,0x2,true);
		spi(myusb,0x80000433,0x3,true);
		spi(myusb,0x800004aa,0x4,true);
		spi(myusb,0x800004f0,0x5,true);
		spi(myusb,0x8000040f,0x6,true);
		spi(myusb,0x80000411,0x7,true);
		spi(myusb,0x80000455,0x8,true);
		spi(myusb,0x800004ff,0x9,true);
		spi(myusb,0x80000c00,0xa,true);

		spi(myusb,0xc05);

		//write burst test
//		int size=128+6;

		spi(myusb,0x80000806,0,true);
		spi(myusb,0xc05);

		int size=8+6;
		unsigned int *bbuf=myusb.getBuffer(size);
		if(!bbuf){
				ASSERT_TRUE(false) << "Alloc buffer failed.";
		}
//		bbuf[0]=__builtin_bswap32(0x80000806);bbuf[1]=__builtin_bswap32(0);
		bbuf[0]=__builtin_bswap32(0x80000402);bbuf[1]=__builtin_bswap32(0x48);
//    bbuf[2]=__builtin_bswap32(0x80000402);bbuf[3]=__builtin_bswap32(0x48);
    bbuf[2]=__builtin_bswap32(0x80000400);bbuf[3]=__builtin_bswap32(16);
		bbuf[4]=__builtin_bswap32(0x80000400);bbuf[5]=0;
		for(int i=6;i<size-2;i+=2){
			bbuf[i]=__builtin_bswap32(0x80000400+i);bbuf[i+1]=__builtin_bswap32(i+1);
		}
		bbuf[size-2]=__builtin_bswap32(0x80000c00+size-2);bbuf[size-1]=__builtin_bswap32(size-1);

//		for(int i=0;i<8*64;i++){
		for(int i=0;i<1;i++){
			printf("%d ",i);
			myusb.burstwriteocp(size);
			addr=0xc05;	
			do myusb.raw(usbcomm::READOCP,addr,result);
			while(result&1);
		}
		printf("\n");

		spi(myusb,0x80000403,0x48,true);
		spi(myusb,0x80000400,0x0,true);

		addr=0x0;
		numrw=128;
		for(int i=0;i<numrw;i++){
			if(i==numrw-1)addr=0x800; //release spi cs
			if(myusb.raw(usbcomm::READOCP,addr,result)){printf("Raw failed\n");ASSERT_EQ(0, myusb.status); return;;
			}else printf("%02x ", result);
			if(!((i+1)%16))printf("\n");			
		} 


/*
		//write whole rom
		const string hexname("bitfile.mcs");
		printf("Hallo\n");
		int error=writerom(myusb,hexname);
		printf("Result of write: %d\n",error);
*/
	}
	//no known bugs at the moment


