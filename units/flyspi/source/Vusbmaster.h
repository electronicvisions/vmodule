#include "vmsp6.h"
#include "vmodule.h"

class Vusbmaster:public Vmodule<sp6adr,sp6data> {
public:
	const static ts_t USB_BS=128; //usb block size in ints: 128
	enum usbmasterid {ID_STATUS=0,ID_MEMORY=1,ID_OCP=2};
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
	enum adrflags {ADR_OCPFIFO=0x8000000}; //adr fields with special definitions
	Vusbmaster(Vmodule<sp6adr,sp6data> *parent):Vmodule<sp6adr,sp6data>(parent){
	};
	ts_t getUsbTS(ts_t num); //calculate USB transfer size
	virtual void initTransfer(uint id, Vrequest<sp6adr> &a);

	virtual void doTransfer(uint id=0);

	//check for ocp fifo entry by directly requesting from parent
	bool readOcpFifo(uint id);
};
