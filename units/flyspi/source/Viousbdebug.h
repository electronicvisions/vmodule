//Viousbdebug simulates usb connection
//simple test: just a loopback

class Viousbdebug:public Vmodule<sp6adr,sp6data> {
	Vbuffer<sp6data> in,out;
public:
	uint statusreg;
	array <uint,10000> mem;
	array <uint,100000> ocpmem; //emulate ocp address space as memory
	Viousbdebug():Vmodule<sp6adr,sp6data>(&out,&in);
//transfer functions overloaded to connect hierarchy to io stream
	void initTransfer(uint id,Vrequest<sp6adr> &a);
	void doTransfer(uint id);
};

