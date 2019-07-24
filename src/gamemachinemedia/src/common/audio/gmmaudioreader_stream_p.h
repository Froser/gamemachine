#ifndef __GMMAUDIOREADER_STREAM_P_H__
#define __GMMAUDIOREADER_STREAM_P_H__
#include <gmmcommon.h>
#include <gminterfaces.h>
#include <gmtools.h>
BEGIN_MEDIA_NS

class GMMStream;
GM_PRIVATE_OBJECT_UNALIGNED(GMMAudioFile_Stream)
{
	// 缓存相关
	gm::GMBuffer fileBuffer;
	gm::GMuint32 bufferNum = 0;
	gm::GMAudioFileInfo fileInfo;
	GMMStream* output = nullptr;

	// stream相关
	gm::GMulong bufferSize = 0;
	bool eof = false;
	gm::GMlong eofPtr = 0;
	GMAtomic<gm::GMlong> writePtr;
	GMAtomic<gm::GMlong> readPtr;
	gm::GMManualResetEvent streamReadyEvent;
	gm::GMManualResetEvent blockWriteEvent;
	GMAtomic<gm::GMlong> chunkNum; //表示当前应该写入多少个chunk
};

END_MEDIA_NS
#endif
