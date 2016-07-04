#include "Vusbmaster.h"
#include "vmodule.h"

ts_t Vusbmaster::getUsbTS(ts_t num){ //calculate USB transfer size
	if(num%USB_BS)return (num/USB_BS+1)*USB_BS;
	else return num;
}

void Vusbmaster::initTransfer(uint id, Vrequest<sp6adr> &a)
{
	requests[id]=a;
	Vrequest<sp6adr> padr;
	switch(id){

		case ID_STATUS:
			padr.out=USB_BS;	//always write one block
			padr.in=USB_BS; //always read one block back
			outbuf->alloc(padr.out);
			if(a.in==0){ //original was write
				getBuffer()->push_back((CMD_WRITESTATUS));
				getBuffer()->push_back(0);//position buffer at data field
			}else{
				getBuffer()->push_back((CMD_READSTATUS));
				getBuffer()->push_back(0);
			}
			break;

		case ID_MEMORY:
			padr.out=getUsbTS(a.out+3);	//always round up to next bs
			padr.in=getUsbTS(a.in+3);
			outbuf->alloc(padr.out);
			if(a.in==0){//write
				getBuffer()->push_back((CMD_WRITEBURST));
				getBuffer()->push_back((a.adr));
				getBuffer()->push_back((a.out));
			}else{
				getBuffer()->push_back((CMD_READBURST));
				getBuffer()->push_back((a.adr));
				getBuffer()->push_back((a.in));
			}
			break;

		case ID_OCP:;
			padr.out=getUsbTS(a.out+3);	//always round up to next bs
			padr.in=getUsbTS(a.in+3);
			outbuf->alloc(padr.out);
			if(a.in==0){//write
				getBuffer()->push_back((CMD_WRITEOCPBURST));
				getBuffer()->push_back((0)); //ocp burst does not use adr
				getBuffer()->push_back((a.out));
			}else{
				if(a.adr==ADR_OCPFIFO){
					getBuffer()->push_back((CMD_READOCPFIFO));
				}else{
					getBuffer()->push_back((CMD_READOCP));
					getBuffer()->push_back((a.adr));
				}
			}
			break;

	}
	Vmodule<sp6adr,sp6data>::initTransfer(id,padr);
}

void Vusbmaster::doTransfer(uint id)
{
	Vmodule<sp6adr,sp6data>::doTransfer(id);
	switch(id){
		case ID_STATUS:
			if(requests[id].in)//read status
				getInbuf()->setPos(2);//status position in stream
			break;
		case ID_MEMORY:
			if(requests[id].in)//read from memory
				getInbuf()->setPos(3);//first read position in stream
			break;
		case ID_OCP:
			if(requests[id].in){//read from ocp fifo
				if((getInbuf()->at(1)) & 0x80000000){
					//fifo is empty, need another readfifo access
					for(uint t=0;t<10;t++)if(!readOcpFifo(id))break;
					//if(t=10)timeout
				}
			}
			getInbuf()->setPos(2);//first read position in stream is data
			break;
	}
}

//check for ocp fifo entry by directly requesting from parent
bool Vusbmaster::readOcpFifo(uint id)
{
	Vrequest<sp6adr> padr;
	padr.out=getUsbTS(3);	//always round up to next bs
	padr.in=getUsbTS(3);
	outbuf->alloc(padr.out);
	getBuffer()->push_back((CMD_READOCPFIFO));
	Vmodule<sp6adr,sp6data>::initTransfer(id,padr);
	Vmodule<sp6adr,sp6data>::doTransfer(id);
	return((getInbuf()->at(1)) & 0x80000000);
}
