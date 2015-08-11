// new electronic visions slow-control base class

using namespace std;

#include <gtest/gtest.h>
#include <list>
#include "vmodule.h"
#include "logger.h"

//Viomodule is the top of the Vmodule tree and connects the buffer to an io device
//in this test case it is cout
//for in-transfers, it just passes back counter values as read data

class Viomodule:public Vmodule<uint,uint> {
public:
	Viomodule(Vbuffer<uint> *out, Vbuffer<uint> *in=nullptr):Vmodule(out,in){}; 
//transfer functions overloaded to connect hierarchy to io stream
	void initTransfer(uint id,Vrequest<uint> &a);
	void doTransfer(uint id);
};

void Viomodule::initTransfer(uint id,Vrequest<uint> &a){
	requests[id]=a;
	getBuffer()->alloc(a.out+1); //one uint adr
	getBuffer()->push_back(a.adr);
}

//in doTransfer the buffer contains adr and data fields, so dotransfer copies the
//buffer to cout and resets it
void Viomodule::doTransfer(uint id){
	for(int i: *getBuffer())
		cout<<i;
	getBuffer()->clear();

	//generate dummy return data as requested
	if(requests[id].in){
		for(uint i=0;i< requests[id].in;i++)inbuf->push_back(i);
	}
}

//add hw-dependent modules
//ocpmodule is base class for all ocp clients
class Vocpmodule:public Vmodule<uint,uint> {
	uint baseadr;
	public:
	Vocpmodule(Vmodule<uint,uint> *parent,uint badr):Vmodule(parent),baseadr(badr){};
	//add base address
	virtual void processAdr(Vrequest<uint> &a){
		a.adr += baseadr;
	};
};

//ocpfifo is parent of all ocpmodules
//ocpfifo changes the buffer pointer for all dependent classes
//and translates from ocp requests to the ocp fifo format 
class Vocpfifo:public Vmodule<uint,uint> {
public:
	Vocpfifo(Vmodule<uint,uint> *parent):Vmodule(parent){
		//change in and out pointers
		inbuf=new Vbuffer<uint>;
		outbuf=new Vbuffer<uint>;
	};
	virtual ~Vocpfifo(){delete inbuf;delete outbuf;};
	virtual void initTransfer(uint id, Vrequest<uint> &a){
		requests[id]=a;
		Vrequest<uint> padr; //request for parent
		padr.adr=a.adr;
		padr.out=2*a.out;//each ocp request needs two uints in fifo
		padr.in=2*a.in;
		getBuffer()->alloc(padr.out);//alloc fifo entries
		Vmodule<uint,uint>::initTransfer(id,padr);
	}	
 
	virtual void doTransfer(uint id=0){
		//convert to ocpfifo
		uint oidx=getBuffer()->size()-requests[id].out;
		uint adr=requests[id].adr;
		for(uint i=0;i<requests[id].out;i++){
			parent->getBuffer()->push_back(adr++);//autoincrement adr
			parent->getBuffer()->push_back( (*getBuffer())[oidx+i] );
		}
		Vmodule<uint,uint>::doTransfer(id=0);
		for(uint i=1;i<requests[id].in*2;i+=2){ //skip address entries
			getInbuf()->push_back((*(parent->getInbuf()))[i]);
		}	
	};
};

//usbmaster manages the usb packet framing
//it has fixed child id -> function assignments
//id0: status register -> class Vusbstatus
//id1: memory						-> class Vddr3ram
//id2: ocp						-> class Vocpfifo

class Vusbmaster:public Vmodule<uint,uint> {
public:
	enum usbmasterid {status=0,memory=1,ocp=2};
	enum usbcommands {
		CMD_NOP,			
		CMD_READMEM,		//obsolete
		CMD_WRITEMEM,		//obsolete
		CMD_READBURST,	//cmd,adr,count,data0,...
		CMD_WRITEBURST, //cmd,adr,count,data0,...
		CMD_READSTATUS, //cmd,dc,status
		CMD_WRITESTATUS,//cmd,dc,status
		CMD_READOCP,		//cmd,adr,data
		CMD_WRITEOCP,		//obsolete
		CMD_READOCPFIFO,//cmd,dc,data
		CMD_WRITEOCPBURST //cmd,dc,count,adr0,data0,...
	};
	Vusbmaster(Vmodule<uint,uint> *parent):Vmodule(parent){
	};
	virtual void initTransfer(uint id, Vrequest<uint> &a){
		requests[id]=a;
		Vrequest<uint> padr;
		switch(id){
			case status:
				padr.out=512;	//always write one block
				padr.in=512; //always read one block back
				outbuf->alloc(padr.out); 
				if(a.in==0) //original was write
					getBuffer()->push_back(CMD_WRITESTATUS);
				else
					getBuffer()->push_back(CMD_READSTATUS);
		}
		Vmodule<uint,uint>::initTransfer(id,padr);
	}	
 
	virtual void doTransfer(uint id=0){
		switch(id){
			
		}
		Vmodule<uint,uint>::doTransfer(id);
		switch(id){
			case status:
				if(requests[id].in){//read status
					getInbuf()->setPos(2);//status position in stream
	
				}
			
		}
	};
};

//usb status class is simple and just reads/writes one word
class Vusbstatus:public Vmodule<uint,uint> {
	public:
	enum status_reg {};
	Vusbstatus(Vmodule<uint,uint> *parent):Vmodule(parent){};
	//HAL functions
	uint getStatus(){
		Vrequest<uint> r(0,0,1);
		initTransfer(0,r);
		doTransfer();
		return *(getInbuf()->begin());//read one entry from buffer
	};
	void setStatus(uint s){
		Vrequest<uint> r(0,1,0);
		initTransfer(0,r);
		getBuffer()->push_back(s);
		doTransfer();
	};
};



//example
TEST(Vmodule, IDontKnowWhatItDoes)
{

Vbuffer<uint> in,out,result;
//create the top of the tree
Viomodule io(&out,&in);
//create three classes
Vmodule<uint,uint> top(&io),child1(&top),child2(&top);

uint adr1=1,adr2=2;
//write to both
Vrequest<uint> r(adr1,1);
child1.initTransfer(0,r );
child1.getBuffer()->push_back(42);
child1.doTransfer();
r.adr=adr2;
child2.initTransfer(0,r );
child2.getBuffer()->push_back(11);
child2.doTransfer();
//read
r.out=0;r.in=5;
child2.initTransfer(0,r );
child2.doTransfer();
child2.getInbuf()->setPos(2);
for(int i: *(child2.getInbuf()) )cout<<i;

}
