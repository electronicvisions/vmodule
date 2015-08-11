#pragma once

#include <vector>

//Vbufptr points to a Vbuffer object
//it gets a reference to a Vbuffer object in the constructor and allows access to
//the data in the buffer from the actural Vbuffer position pos to the current allocated
//maximum buf.capacity
//while it exists, the Vbuffer is locked
template <typename DATA>
class Vbufptr {
	Vbuffer<DATA> *buf;
	typename std::vector<DATA>::iterator p;
	public:
	explicit Vbufptr(Vbuffer<DATA> &b):buf(&b){
		p=buf->begin();
		//!!!insert check of buffer already locked here
		buf->lock();
	}
	//same as above, but with vbuffer * instead of reference
	explicit Vbufptr(Vbuffer<DATA> *b):buf(b){
		p=buf->begin();
		//!!!insert check of buffer already locked here
		buf->lock();
	};
	//copy constructor implements move, source must be allocated on heap!!!
	Vbufptr(Vbufptr const &src):buf(src.buf){
		p=buf->begin();

	}
	~Vbufptr(){if(buf)buf->unLock();}
	inline DATA & operator[](ts_t pos){return p[pos];};
	inline DATA & operator*(){return *p;};
	inline Vbufptr & operator++(){++p;};
	inline Vbufptr & operator++(int){p++;};
};
