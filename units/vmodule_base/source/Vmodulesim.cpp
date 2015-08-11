/*
	simulator interaction module
	Andreas Hartel 2012
*/

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <cstring>
#include <list>

#include "Vmodulesim.h"
#include "vmodule.h"
#include "vmsp6.h"
#include "logging_ctrl.h"

void Vmodulesim::initTransfer(uint id,Vrequest<sp6adr> &req){
    requests[id]=req;
    getBuffer()->alloc(requests[id].out);
};

void Vmodulesim::doTransfer(uint id){
	Logger& log = Logger::instance();
    ubyte *out,*in;
	out=(ubyte*)getBuffer()->data();
	getInbuf()->clear();
	getInbuf()->alloc(requests[id].in);
	in=(ubyte*)getInbuf()->data();

	log(Logger::DEBUG0) << "Output packet size: " << requests[id].out*sizeof(sp6data);

	size_t size = requests[id].out*sizeof(sp6data);
	{
		vector<char> send_vector(size,0);
		log(Logger::DEBUG0) << "outbuf:";
		stringstream strm;
		for (int i=0; i<size; i++)
		{
			// cout stuff (ignore)
			if (i%16==0 && i>0)
			{
				log(Logger::DEBUG0) << strm.str();
				strm.str("");
			}
			if (out[i]<16) strm << "0";
			strm << hex << int(out[i]);
			if ((i+1)%4==0 && (i+1)%16!=0) {
				strm << " | ";
			}
			else {
				strm << " ";
			}
			// actual assignment
			send_vector[i] = out[i];
		}
		log(Logger::DEBUG0) << "";

		flansch.send_custom_packet(send_vector);
	}

	getBuffer()->clear(); //buffer is sent

	log(Logger::DEBUG0) << "Input packet size: " << dec << requests[id].in*sizeof(sp6data);

	size = requests[id].in*sizeof(sp6data);
	{
		std::vector<char> tmp = flansch.get_custom_packet();

		log(Logger::DEBUG0) << "inbuf:";
		stringstream strm;
		for (int i=0; i<size; i++) {
			// actual assignment
			in[i] = tmp[i];
			// cout stuff (ignore)
			if (i%16==0 && i>0)
			{
				log(Logger::DEBUG0) << strm.str();
				strm.str("");
			}
			if ((tmp[i]>>4)==0) strm << "0";
			strm << hex << int(in[i]);
			if ((i+1)%4==0 && (i+1)%16!=0) {
				strm << " | ";
			}
			else {
				strm << " ";
			}
		}
		log(Logger::DEBUG0) << "";
	}
};

