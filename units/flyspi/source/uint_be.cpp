
//big-endian uint32 class to handle buffer data
uint_be::uint_be(uint nd)
{
	d = __builtin_bswap32(nd);
}; //used to creat uint_le from uint

//uint assignment
void uint_be::operator=(const uint &nd)
{
	d=__builtin_bswap32(nd);
};

//conversion to uint
operator uint_be::uint() const
{
	return __builtin_bswap32(d);
};
