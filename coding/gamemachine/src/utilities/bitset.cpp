#include "stdafx.h"
#include "bitset.h"
#include "assert.h"

bool Bitset::init(int numberOfBits)
{
	//Delete any memory allocated to bits
	if (bits)
		delete[] bits;
	bits = NULL;

	//Calculate size
	numBytes = (numberOfBits >> 3) + 1;

	//Create memory
	bits = new unsigned char[numBytes];
	if (!bits)
	{
		gm_error("Unable to allocate space for a Bitset of %d bits", numberOfBits);
		return false;
	}

	clearAll();

	return true;
}

void Bitset::clearAll()
{
	memset(bits, 0, numBytes);
}

void Bitset::setAll()
{
	memset(bits, 0xFF, numBytes);
}

void Bitset::clear(int bitNumber)
{
	bits[bitNumber >> 3] &= ~(1 << (bitNumber & 7));
}

void Bitset::set(int bitNumber)
{
	bits[bitNumber >> 3] |= 1 << (bitNumber & 7);
}

unsigned char Bitset::isSet(int bitNumber)
{
	return bits[bitNumber >> 3] & 1 << (bitNumber & 7);
}
