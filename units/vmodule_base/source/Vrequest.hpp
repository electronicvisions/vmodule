#pragma once

//Vrequests queues a read request
template <typename ADR>
struct Vrequest {
	Vrequest():adr(0),in(0),out(0){};
	Vrequest(ADR a,ts_t tout, ts_t tin=0):adr(a),in(tin),out(tout){};
	ADR adr;
	ts_t in; //number of requested read entries
	ts_t out; //number of write transfers
};
//Vmodule represent either a data sink or source or a piece of the hw communication
//chain which changes the data stream (i.e. adds/removes header information)
