#pragma once
#include "test.h"

//vbuffer handles the data streams to achieve zero copy behavior throughout the
//hardware tree
template <typename DATA>
class Vbuffer {
	std::vector<DATA> buf;
	bool locked;
	ts_t pos; //start position of free data area
	public:
	const static uint VBUF_RESERVE=1000;
	Vbuffer():locked(false),pos(0){};
	DATA * data(){return buf.data();};
	DATA * firstFree(){return (buf.data()+buf.size());}
	DATA * posPtr(){return (buf.data()+pos);};
	void alloc(ts_t size){
		if(buf.size()+size >= buf.capacity())buf.reserve(buf.size()+size+VBUF_RESERVE);
	};
	inline ts_t getPos(){return pos;};
	inline void movePos(){pos=buf.size();}; //now points to first free entry
	inline void setPos(ts_t p){pos=p;};

	inline void push_back(const DATA & d){buf.push_back(d);movePos();};

	inline DATA & operator[](ts_t p){return buf[p];};
	inline typename std::vector<DATA>::iterator begin(){return buf.begin()+pos;};
	inline typename std::vector<DATA>::iterator end(){return buf.end();};
	inline ts_t size(){return buf.size();};
	inline DATA & at(ts_t p){//check against buf.capacity, not buf.size!!!
		//insert range check here
		return buf[p];
	};
	void clear(){buf.clear();pos=0;};
	//lock/unlock, return treu if successful
	inline bool lock(){if(locked==false){locked=true;return true;} else return false; };
	inline bool unLock(){if(locked==true){locked=false; return true;} else return false; };
	inline bool getLock(){return locked;};

	FRIEND_TEST(Vmodule, IDontKnowWhatItDoes);
};
