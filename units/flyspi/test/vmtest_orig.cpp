/*
	vmodule sp6board usage example
	Johannes Schemmel 2011
*/


// new electronic visions slow-control base class

using namespace std;

#include <cstring>
#include <list>
#include <gtest/gtest.h>

#include "logger.h"

#include "vmodule.h"
#include "Vusbmaster.h"
#include "Vusbstatus.h"
#include "Vmemory.h"
#include "vmsp6.h"
#include "Vmoduleusb.h"
#include "Vocpfifo.h"
#include "Vspiconfrom.h"
#include "Vspigyro.h"
#include "Vspiwireless.h"
#include "Vspifastadc.h"

int64_t memtest_orig( Vmemory& mem, ts_t testsize, uint loops) {
	int64_t errors=0;
	const uint bufsize=0x200000;//124;//0x200000;
	int blocks;
	int size;
	int base=0;        //for second memory block: 1<<27;
	//caluclate counter limits
	if(testsize<bufsize){size=testsize;blocks=1;}
	else{size=bufsize;blocks=testsize/bufsize;}
	int seed=time(NULL);

	for(uint l=0;l<loops;l++){
		cout<<dec<<"Memory testloop "<<l<<endl;
		//	int size = 0x200000; //memory buffer allocated on heap in vbuffer
		//	int blocks = 1; //64;    //16*256;//(0x4000000-1) / size; //64 M Langworte
		int adr=base; //second memory
		
		srand(seed);	
		printf("Writeburst\n");
		for(int l=0;l<blocks;l++){
			Vbufuint_p buf=mem.writeBlock(adr,size);
			if(!(l%1))printf("%08x\n",adr*4);
			for(int j=0;j<size;j++)buf[j]=rand();
			mem.doWB();
			adr+=size;
			if(adr>0x3ffffff && adr<(1<<27))adr=1<<27; //switch to second memory block
		}

		printf("Readburst\n");
		//	blocks=16;
		//size=0x1;
		adr=base;
		srand(seed);
		for(int l=0;l<blocks;l++){
			if(!(l%1))printf("%08x\n",adr*4);
			Vbufuint_p buf=mem.readBlock(adr,size); //buffer is locked while buf exists
			for(int j=0;j<size;j++){
				uint test=rand();
				if(test!=buf[j]){
					if(errors<100)printf("%08x %08x %08x\n",(j+adr)*4,test,(uint)buf[j]);
					errors++;
				}
			}
			adr+=size;
			if(adr>0x3ffffff && adr<(1<<27))adr=1<<27; //switch to second memory block
		}
	cout<<dec<<"Errors so far: "<<errors<<endl;
							
	}
	return errors;
}


TEST(VMTest, original)
{
//select loggin level 
Logger::instance("vmodule.test.VMTest",Logger::INFO,"");


//create the top of the tree
//Viousbdebug io;
Vmoduleusb io(usbcomm::note);

if(io.open(0x04b4,0x1003)){
	cout<<"Open failed"<<endl;exit(-1);
}else cout<<"Board opened"<<endl;

//create sp6 class tree
Vusbmaster usb(&io);
//usbmaster knows three clients, must be in this order!!!
Vusbstatus status(&usb);
Vmemory mem(&usb);
Vocpfifo ocp(&usb);
//ocpfifo clients
Vspiconfrom confrom(&ocp);
Vspigyro gyro(&ocp);
Vspiwireless wireless(&ocp);
Vspifastadc spiadc(&ocp);
//Vfastadc adc(&ocp);

//know we can do io like we always wanted
//cout<<status.getStatus()<<endl;
//status.setStatus(Vusbstatus::c1_selfrefresh_enter|Vusbstatus::c3_selfrefresh_enter);
status.setStatus(0);
cout<<status.getStatus()<<endl;

//some memory io
mem.write(0,0x33333333);
mem.write(1,0xaaaaaaaa);
cout<<hex<<mem.read(1)<<" "<<mem.read(0)<<endl;

//memtest(mem,1<<27,1);
for (unsigned int i=128; i<16*1024*1024+1; i=i*2)
	memtest_orig(mem,i,10);

//ubyte test[Vspiconfrom::SECSIZE];

//for(uint i=0;i<Vspiconfrom::SECSIZE;i++)test[i]=i;
/*
uint secadr=0;
int numread=256;
vector<ubyte> buf;
confrom.read_mem(secadr,&buf,numread);
for(int i=0;i<numread;i++){
	printf("%02x ", buf[i]);
	if(!((i+1)%16))printf("\n");			
}
*/

//confrom.write_sector(secadr,test);
 
/*confrom.read_mem(secadr,buf,numread);
for(int i=0;i<numread;i++){
	printf("%02x ", buf[i]);
	if(!((i+1)%16))printf("\n");			
}

confrom.read_mem(0,buf,numread);
for(int i=0;i<numread;i++){
	printf("%02x ", buf[i]);
	if(!((i+1)%16))printf("\n");			
} */

/*
string hexname;
cin >> hexname;
int error=confrom.program(hexname);
printf("Result of write: %d\n",error);
*/

/*
//some ocp memory   
ocp.write(5,0x12345678);
ocp.write(16,0x33333333);
cout<<hex<<ocp.read(5)<<" "<<ocp.read(16)<<endl;
//run adc
spiadc.write(0,0x10);
spiadc.write(0xb,0x154);
adc.run(4,8,Vfastadc::adc_reset_n|Vfastadc::adc_pdn_n|Vfastadc::cb1);
cout<<"adc: "<<hex<<adc.read(3)<<endl;
//wireless
wireless.write(0x1d,0xaa);
cout<<"wireless: "<<hex<<(uint)wireless.read(0x1d)<<endl;
//gyro
gyro.write(0x2f,0x33);
cout<<"gyro: "<<hex<<(uint)gyro.read(0x2f)<<endl;
//config
cout<<"confrom: "<<hex<<(uint)confrom.read(5)<<endl;
ubyte data[10];
confrom.read_mem(5,data,10);

*/

}
