#ifndef __GMMAUDIOREADER_STREAM_H__
#define __GMMAUDIOREADER_STREAM_H__
#include <gmmcommon.h>
#include <atomic>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gmtools.h>
BEGIN_MEDIA_NS

// 所有流文件的基类，支持多线程解码，编码写入以及线程同步

GM_PRIVATE_OBJECT_UNALIGNED(GMMAudioFile_Stream);
class GM_MEDIA_EXPORT GMMAudioFile_Stream : public gm::IAudioFile, public gm::IAudioStream
{
	GM_DECLARE_PRIVATE(GMMAudioFile_Stream)
	GM_DISABLE_COPY_ASSIGN(GMMAudioFile_Stream)

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
	virtual gm::GMuint32 getBufferId() override;

	// gm::IAudioStream
public:
	virtual gm::GMuint32 getBufferNum() override;
	virtual gm::GMuint32 getBufferSize() override;
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
	static void move(std::atomic_long& ptr, gm::GMuint32 loop);
	static gm::GMlong peek(std::atomic_long& ptr, gm::GMuint32 loop);

public:
	static void setStreamReady(Data* d);
	static void setEof(Data* d);

private:
	void init();
	void cleanUp();
};

END_MEDIA_NS
#endif
