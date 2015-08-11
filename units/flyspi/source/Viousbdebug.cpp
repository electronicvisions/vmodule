#include "Viousbdebug.h"

Viousbdebug::Viousbdebug():Vmodule<sp6adr,sp6data>(&out,&in)
{
}

void Viousbdebug::initTransfer(uint id,Vrequest<sp6adr> &a)
{
    requests[id]=a; //remember request
}

//in doTransfer the buffer contains adr and data fields, so dotransfer copies the
//buffer to cout and resets it
void Viousbdebug::doTransfer(uint id)
{
    getInbuf()->clear();
    cout<<getBuffer()->size()<<" "<<requests[id].out<<" "<<requests[id].in<<endl;

    unsigned char *cbuf=(unsigned char *)(getBuffer()->data()); //start at buffer begin
    for(ts_t i=0; i<requests[id].out*4; i++) //loop over bytes
    {
        printf("%02x ",cbuf[i]);
        if((i+1)%16==0)
        {
            cout<<endl;
        };
    }
    for(ts_t i=0; i<requests[id].in; i++)
        getInbuf()->push_back( (*getBuffer())[i] );	//generate dummy return data

    //interpret packet and emulate part of the hw
    sp6data *obuf=getBuffer()->data();
    sp6data *ibuf=getInbuf()->data();
    uint cmd=(obuf[0]);
    uint adr=(obuf[1]);
    uint data=(obuf[2]);
    switch(cmd)
    {
    case Vusbmaster::CMD_READSTATUS:
        ibuf[2]=(statusreg);
        break;
    case Vusbmaster::CMD_WRITESTATUS:
        statusreg=data;
        break;
    case Vusbmaster::CMD_READBURST:
        for(uint i=0; i<data; i++)ibuf[3+i]=mem.at(adr+i);
        break;
    case Vusbmaster::CMD_WRITEBURST:
        for(uint i=0; i<data; i++)mem.at(adr+i)=obuf[3+i];
        break;
    case Vusbmaster::CMD_WRITEOCPBURST:
        for(uint i=0; i<data/2; i++)
            ocpmem.at((obuf[3+i*2]) & 0x7fffffffU)=obuf[4+i*2];
    case Vusbmaster::CMD_READOCP:
        //emulate timeout: all odd addresses have empty fifo
        ibuf[2]=ocpmem.at(adr);
        if(adr%2)ibuf[1] = ibuf[1] | (0x80000000);
        break;
    case Vusbmaster::CMD_READOCPFIFO:
        ibuf[2]=ocpmem.at(adr);
        break;
    }

    unsigned char *icbuf=(unsigned char *)(getInbuf()->data()); //start at buffer begin
    for(ts_t i=0; i<requests[id].in*4; i++) //loop over bytes
    {
        printf("%02x ",icbuf[i]);
        if((i+1)%16==0)
        {
            cout<<endl;
        };
    }

    getBuffer()->clear();
}
