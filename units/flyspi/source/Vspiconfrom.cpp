#include "Vspiconfrom.h"
#include "Vusbmaster.h"
#include "readhex.h"

//single byte read/write
ubyte Vspiconfrom::read(ubyte adr, bool useruseadr)
{
	return Vspimodule::read(adr&0xff,useruseadr);
}

void Vspiconfrom::write(ubyte adr, ubyte data, bool useruseadr)
{
	Vspimodule::write(adr&0xff,data,useruseadr);
}

sp6data Vspiconfrom::get_device_id ()
{
	/* This function returns the manufacturer
	 * and device ID of the PROM chip.
	 * The lower 8 bits of the return value are
	 * the device ID and the next 8 bits are 
	 * the manufacturer ID.
	 *
	 * Manufacturer IDs are:
	 * 0xEF - Winbond
	 * 0xBF - SST
	 * 0x20 - ST Microelectronics
	 */
	sp6data *b=writeBlock(0,8); //adr field has no meaning, 2 entries
	// write read command
	*(b++) = ((ocpwrite | useadr | cmd_rdid) + baseadr);
	*(b++) = 0;
	*(b++) = (baseadr);
	*(b++) = 0xaffe;
	*(b++) = (baseadr);
	*(b++) = 0xaffe;
	*(b++) = (baseadr | esc);
	*(b++) = 0xaffe;
	doWB();
	// read back data
	ubyte vmsb = (ubyte)Vocpmodule::read(Vusbmaster::ADR_OCPFIFO);
	ubyte msb = (ubyte)Vocpmodule::read(Vusbmaster::ADR_OCPFIFO);
	ubyte lsb = (ubyte)Vocpmodule::read(Vusbmaster::ADR_OCPFIFO);
	return (vmsb<<16) | (msb<<9) | lsb;
}

//multi-byte read/write functions
//read a block from config rom
void Vspiconfrom::read_mem(uint adr,ubyte *data,ts_t num)
{
	Vrequest<sp6adr> r(0,num*2+4,0);
	initTransfer(0,r);
	sp6data *b=getBuffer()->posPtr();
	*b++=(ocpwrite|baseadr|cmd_read|useadr);
		*b++=((adr>>16)&0xff);
	*b++=(ocpwrite|baseadr|useadr|((adr>>8)&0xff) );
		*b++=(adr&0xff);
	for(uint i=0;i<num-1;i++){//send opc read cmds to spi controller
		*b++=(baseadr);*b++=(42);
	}
	//last read with esc to release spi cs
		*b++=(baseadr|esc);*b++=(0);
	doTransfer();
	//now send num readfifo cmds
	for(uint i=0;i<num;i++)data[i]=(ubyte)Vocpmodule::read(Vusbmaster::ADR_OCPFIFO);
}

//writes a sector and reprograms it, polls for erase completion, blocks and is slow!!!
//sector size is 64k
void Vspiconfrom::write_sector(uint startadr, ubyte *data)
{
	uint sector=(startadr>>16)&0xff;
	write(cmd_wren,0,false); //write enable
	//erase sector command
	Vrequest<sp6adr> r(0,6,0);
	initTransfer(0,r);
	sp6data *b=getBuffer()->posPtr();
	*b++=(cmd_se|useadr|baseadr|ocpwrite);*b++=(sector);
	*b++=(ocpwrite|baseadr);*b++=0;
	*b++=(ocpwrite|baseadr|esc);*b++=0;
	doTransfer();
	//wait for sector erased
	uint cnt=0;
	while(read(cmd_rdsr)&1){cnt++;} //poll for no erase pending
	mLog(Logger::INFO)<<"Sektor "<<sector<<" erased. Polls: "<<cnt;
	//write loop
	for(uint wa=0;wa<SECSIZE;wa+=128){
		Vrequest<sp6adr> ws(0,128+6,0);
		initTransfer(0,ws);
		b=getBuffer()->posPtr();
		*b++=(ocpwrite|baseadr|esc|cmd_wren);*b++=0;
		*b++=(ocpwrite|baseadr|useadr|cmd_pp);*b++=(sector);
		*b++=(ocpwrite|baseadr|useadr|((wa>>8)&255));*b++=(wa&255);
		for(int i=0;i<63;i++){//write 126 bytes in the loop
			*b++=(ocpwrite|baseadr|useadr| *data++);*b++=(*data++);
		}
		*b++=(ocpwrite|baseadr|useadr|esc| *data++); //end spi command
		*b++=(*data++);
		doTransfer();
		cnt=0;
		while(read(cmd_rdsr)&1){cnt++;}; //poll for no write pending
		if(cnt>1)mLog(Logger::DEBUG0)<<std::dec<<"Polls: "<<cnt<<" for adr: "<<std::hex<<(startadr+wa);//only log if write takes unusual long
	}
	mLog(Logger::INFO)<<"Sektor "<<std::dec<<sector<<" programmed.";
}

int Vspiconfrom::program(const std::string &name)
{
    //Logger &log=Logger::instance(Logger::INFO,"");

    mLog(Logger::INFO)<<"Reading config file...";
    const uint secsize=0x10000; //64kB sectors
    unsigned char hexbuf[1024*1024*4] ;//max 32MBit flash !!!could not allocate enough memory
    int numread=readhex(name.c_str(),hexbuf,sizeof(hexbuf));
    if(numread<0)
    {
        mLog(Logger::WARNING)<<"readhex failed: "<<numread;
        return numread;
    }
    uint numsec=numread/secsize;
    if(numread%secsize)numsec++; //only program whole sectors

    mLog(Logger::INFO)<<"Burning "<<name<<" to config rom. Size: "<<numread<<" Sectors: "<<numsec<<Logger::flush;
    unsigned char *b=hexbuf; //data to burn

	// Check which prom we're talking to
	switch(get_device_id())
	{
		// SST
		case 0x2596BF:
			mLog(Logger::INFO)<<"Found SST Flash chip, manufacturer ID 0xBF" << Logger::flush;
			write(cmd_wren,0,false);
			write(cmd_wrsr,0);
			break;
		// ST Microelectronics
		case 0x202e10:
			mLog(Logger::INFO)<<"Found ST Microelectronics Flash chip, manufacturer ID 0x20" << Logger::flush;
			break;
		default:
			break;
	}

    for(uint sec=0; sec<numsec; sec++)
    {
        write_sector(sec*SECSIZE,b+sec*SECSIZE);
    }

    return 0;
}
