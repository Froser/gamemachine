#ifndef __GMMSTREAM_H__
#define __GMMSTREAM_H__

#include <gmmcommon.h>
#include <gamemachine.h>

BEGIN_MEDIA_NS

GM_PRIVATE_OBJECT(GMMStream)
{
	gm::GMbyte* data = nullptr;
	gm::GMEvent preventRead;
	gm::GMuint ptr = 0;
	gm::GMuint capacity = 0;
};

class GMMStream : public gm::GMObject
{
	DECLARE_PRIVATE(GMMStream)

public:
	GMMStream() = default;
	~GMMStream();

public:
	void resize(size_t sz);
	void beginWrite();
	void endWrite();
	void rewind();
	bool isFull();

public:
	GMMStream& operator <<(gm::GMbyte byte);
	bool read(gm::GMbyte* buffer);
};

END_MEDIA_NS

#endif //__GMMAUDIOPLAYER_H__