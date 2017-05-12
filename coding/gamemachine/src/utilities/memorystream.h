#ifndef __MEMORYSTREAM_H__
#define __MEMORYSTREAM_H__
#include "common.h"
BEGIN_NS

class MemoryStream
{
public:
	enum SeekMode
	{
		FromStart,
		FromNow,
	};

public:
	MemoryStream(const GMbyte* data, GMuint size);

public:
	GMuint read(GMbyte* buf, GMuint size);
	GMuint peek(GMbyte* buf, GMuint size);
	void seek(GMuint cnt, SeekMode = FromStart);
	void rewind();
	GMuint size();
	GMuint tell();
	GMbyte get();

private:
	const GMbyte* m_ptr;
	const GMbyte* m_start;
	const GMbyte* m_end;
	GMuint m_size;
};

END_NS
#endif