#ifndef __BITSET_H__
#define __BITSET_H__
#include "common.h"
BEGIN_NS

class Bitset
{
public:
	Bitset() : numBytes(0), bits(NULL)
	{}
	~Bitset()
	{
		if (bits)
			delete[] bits;
		bits = NULL;
	}

	bool init(int numberOfBits);
	void clearAll();
	void setAll();

	void clear(int bitNumber);
	void set(int bitNumber);

	unsigned char isSet(int bitNumber);

protected:
	GMint numBytes;	//size of bits array
	unsigned char* bits;
};

END_NS
#endif