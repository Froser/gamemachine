#ifndef __GMMAUDIOREADER_STREAM_H__
#define __GMMAUDIOREADER_STREAM_H__
#include <gmmcommon.h>
#include <atomic>
#include <gamemachine.h>
BEGIN_MEDIA_NS

// 所有流文件的基类，支持多线程解码，编码写入以及线程同步
class GMMStream;
GM_PRIVATE_OBJECT(GMMAudioFile_Stream)
{
	// 缓存相关
	gm::GMuint bufferNum = 0;
	gm::GMAudioFileInfo fileInfo;
	GMMStream* output = nullptr;

	// stream相关
	gm::GMulong bufferSize = 0;
	std::atomic_long writePtr;
	std::atomic_long readPtr;
	gm::GMManualResetEvent streamReadyEvent;
	gm::GMManualResetEvent blockWriteEvent;
	std::atomic_long chunkNum; //表示当前应该写入多少个chunk
};

class GMMAudioFile_Stream : public gm::IAudioFile, public gm::IAudioStream
{
	DECLARE_PRIVATE(GMMAudioFile_Stream)

public:
	GMMAudioFile_Stream();
	~GMMAudioFile_Stream();
public:
	bool load(gm::GMBuffer& buffer);

	// gm::IAudioFile
public:
	virtual bool isStream() override;
	virtual gm::IAudioStream* getStream() override;
	virtual gm::GMAudioFileInfo& getFileInfo() override;
	virtual gm::GMuint getBufferId() override;

	// gm::IAudioStream
public:
	virtual gm::GMuint getBufferNum() override;
	virtual gm::GMuint getBufferSize() override;
	virtual bool readBuffer(gm::GMbyte* bytes) override;
	virtual void nextChunk(gm::GMlong chunkNum) override;
	virtual void rewind() override;

protected:
	void waitForStreamReady();

protected:
	virtual void rewindDecode();
	virtual void startDecodeThread() = 0;

protected:
	static void saveBuffer(Data* d, gm::GMbyte data);
	static void move(std::atomic_long& ptr, gm::GMuint loop);

public:
	static void setStreamReady(Data* d);

private:
	void cleanUp();
};

END_MEDIA_NS
#endif
