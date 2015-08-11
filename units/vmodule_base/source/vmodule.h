/*
	vmodule header
	Johannes Schemmel 2011
*/
#ifndef VMODULEH
#define VMODULEH

// new electronic visions slow-control base class

#include <vector>
#include <array>
#include "logger.h"
#include "test.h"

//some macros
#define unused(x) static_cast<void>((x))

//some types
typedef unsigned int ts_t;
typedef unsigned char ubyte;

#include "Vrequest.hpp"
#include "Vbuffer.hpp"
#include "Vbufptr.hpp"

// Verror object is used for exceptions and error handling

struct Verror {
	enum state {ok, // clear to proceed
		message, // clear to proceed, but something noteworthy happened
		warning, // clear to preceed, but a warning was issued
		error, 	 // last operation failed, take action
		failure}; // catastrophic failure, can not recover, restart

};

template <typename ADR, typename DATA> //do not fix data type yet
class Vmodule : public LoggerMixin {
	public:
	const static uint MAXCHILDS=64;
	protected:
	std::vector<Vmodule<ADR,DATA> *> childs;
	std::array<Vrequest<ADR>, MAXCHILDS> requests; //child requests, uses same index as childs
	std::array<uint,MAXCHILDS> queued; //number of queued requests
	Vmodule<ADR,DATA> * parent;
	uint myid; //handle in parent's child list
	Vbuffer<DATA> *inbuf,*outbuf; //buffers are set in the constructor
//*** data transfer ***
	//get buffer reference, prepare header, never blocks, throws exception if buffer can not be initialized
	virtual void initTransfer(uint id, Vrequest<ADR> &a);
	//call after buffer is filled, might block, throws exception if transfer fails
	virtual void doTransfer(uint id=0);
//*** access functions ***
public:
	//get pointer to data array -> Vbuffer is locked from init to do transfer
	inline Vbuffer<DATA> * getBuffer(uint id=0){unused(id); return outbuf;};
	inline Vbuffer<DATA> * getInbuf(uint id=0){unused(id); return inbuf;};
protected:
//*** polymorphism to modifiy header
	virtual void processAdr(Vrequest<ADR> &/*a*/){};
//the public interface common to all hw modules
	public:
//*** constructor and destructor
	explicit Vmodule():parent(NULL),inbuf(NULL),outbuf(NULL){};
	explicit Vmodule(Vmodule<ADR,DATA> *parent);
	explicit Vmodule(Vbuffer<DATA> *ob, Vbuffer<DATA> *ib=NULL):parent(NULL),inbuf(ib),outbuf(ob){}; //used for top of tree, or if buffer model changes
	virtual ~Vmodule(){};
//*** initialization ***
	virtual void addChild(Vmodule<ADR,DATA> * child);//sets parent in child
	virtual void removeChild(uint /*handle*/){};//destroys child

	FRIEND_TEST(Vmodule, IDontKnowWhatItDoes);
};

class uint_le {
    uint d; //the data
    public:
    uint_le(uint nd){d=__builtin_bswap32(nd);}; //used to creat uint_le from uint
//  uint_le(){};
//uint_le(uint_le &nd):d(nd){};
//uint assignment
    inline void operator=(const uint &nd){d=__builtin_bswap32(nd);};
//conversion to uint
    operator uint() const {return __builtin_bswap32(d);};
};


//tree constructor
template <typename ADR, typename DATA>
Vmodule<ADR,DATA>::Vmodule(Vmodule<ADR,DATA> *p){
	parent = p;
	p->addChild(this);//add child also initializes part of this
}

template <typename ADR, typename DATA>
void Vmodule<ADR,DATA>::addChild(Vmodule<ADR,DATA> *child){
	childs.push_back(child);
	uint childid=childs.size()-1;
	child->myid=childid; //return index to child entry
	child->outbuf=outbuf;
	child->inbuf=inbuf;
	queued[childid]=0; //nothing queued from new child
}

//this functions has to be overloaded to inplement any header processing
template <typename ADR, typename DATA>
void Vmodule<ADR,DATA>::initTransfer(uint /*id*/, Vrequest<ADR> &a){
//pass request
	processAdr(a);
  parent->initTransfer(myid,a);
}

template <typename ADR, typename DATA>
void Vmodule<ADR,DATA>::doTransfer(uint /*id*/){
//send data and wait for result
	parent->doTransfer(myid);
}


#endif
