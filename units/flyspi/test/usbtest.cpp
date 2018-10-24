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
      
      #include <libusb.h>
      
		
TEST(USBTest, IDontKnowWhatItDoes)
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

			 	unsigned int result;
				unsigned int data1=0xaabbeecc,data2=0xdeaddead;
	
				if(myusb.write(0x100,data1)){
					printf("write failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else printf("Write.");
				if(myusb.write(0x101,data2)){
					printf("write failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else printf("Write.");

				if(myusb.read(0x100,result)){
					printf("read failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else {
					printf("Read1: %02x\n", result);
					ASSERT_EQ(result,data1);
				}
				if(myusb.read(0x101,result)){
					printf("read failed\n");ASSERT_EQ(0, myusb.status); return;;
				}else {
				    printf("Read2: %02x\n", result);
					ASSERT_EQ(result,data2);
				}


				int seed=time(NULL);
				//int loops=10;
				int base=1<<27; //second memory
				srand(seed);	
				int test;	
	/*			printf("Writeloop\n");
				for(int j=0;j<loops;j++){
					test=rand();
					myusb.write(j+base,test);
				}
				srand(seed);	
				printf("Readloop\n");
				for(int j=0;j<loops;j++){
					test=rand();
					myusb.read(j+base,result);	
					if(test!=result)printf("%08x %08x %08x\n",(j+base)*4,test,result);
				}
		*/		
				// same with burst
				unsigned int size = 0x200000; //mehr geht nicht?
				unsigned int blocks = 1; //64;    //16*256;//(0x4000000-1) / size; //64 M Langworte
				base=0;            //1<<27;
				int adr=base; //second memory
	
				unsigned int *bbuf= myusb.getBuffer(size);
				ASSERT_TRUE(bbuf) << "Alloc buffer failed.\n";
	
				srand(seed);	
				printf("Writeburst\n");
				for(unsigned int l=0;l<blocks;l++){
					if(!(l%1))printf("%08x\n",adr*4);
					for(unsigned int j=0;j<size;j++)bbuf[j]=rand();
					myusb.burstwrite(adr,size);
					adr+=size;
					if(adr>0x3ffffff && adr<(1<<27))adr=1<<27; //switch to second memory block
				}

				printf("Readburst\n");
//				blocks=16;
//			size=0x1;
				adr=base;
				srand(seed);
				for(unsigned int l=0;l<blocks;l++){
					if(!(l%1))printf("%08x\n",adr*4);
					myusb.burstread(adr,size);
//					myusb.read(adr,result);bbuf[0]=result;
					for(unsigned int j=0;j<size;j++){
						test=rand();
						if(test!=bbuf[j])printf("%08x %08x %08x\n",(j+adr)*4,test,bbuf[j]);
						ASSERT_EQ(test,bbuf[j]);
					}
					adr+=size;
					if(adr>0x3ffffff && adr<(1<<27))adr=1<<27; //switch to second memory block
				}
								
				myusb.selfrefresh(3); //disable all memories

			}
			//no known bugs at the moment
