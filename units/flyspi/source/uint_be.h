
//big-endian uint32 class to handle buffer data
class uint_be {
	uint d; //the data
	public:
	uint_be(uint nd); //used to creat uint_le from uint
//	uint_be(){};
//uint_be(uint_le &nd):d(nd){};
//uint assignment
	inline void operator=(const uint &nd);
//conversion to uint
	operator uint() const;
};

//define template usage in sp6 classes

//typedef uint_le sp6data; 
typedef uint_be sp6data;
typedef uint sp6adr;
typedef Vbufptr<sp6data> Vbufuint_p ;
