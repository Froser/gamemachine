#include "stdafx.h"
#include "gmmaudioreader_stream.h"
#include "common/utilities/gmmstream.h"

GMMAudioFile_Stream::GMMAudioFile_Stream()
{
	D(d);
	d->writePtr = 0;
	d->readPtr = 0;
	d->chunkNum = 0;
}

GMMAudioFile_Stream::~GMMAudioFile_Stream()
{
	D(d);
	if (d->output)
		delete[] d->output;
}

bool GMMAudioFile_Stream::load(gm::GMBuffer& buffer)
{
	D(d);
	d->fileInfo.data = buffer.buffer;
	d->fileInfo.size = buffer.size;
	d->bufferNum = getBufferNum();
	nextChunk(d->bufferNum);
	d->streamReadyEvent.reset();
	startDecodeThread();
	return true;
}

bool GMMAudioFile_Stream::isStream()
{
	return true;
}

gm::IAudioStream* GMMAudioFile_Stream::getStream()
{
	return this;
}

gm::GMAudioFileInfo& GMMAudioFile_Stream::getFileInfo()
{
	D(d);
	waitForStreamReady();
	return d->fileInfo;
}

gm::GMuint GMMAudioFile_Stream::getBufferId()
{
	return 0;
}

gm::GMuint GMMAudioFile_Stream::getBufferNum()
{
	return STREAM_BUFFER_NUM;
}

gm::GMuint GMMAudioFile_Stream::getBufferSize()
{
	D(d);
	waitForStreamReady();
	return d->bufferSize;
}

bool GMMAudioFile_Stream::readBuffer(gm::GMbyte* bytes)
{
	D(d);
	waitForStreamReady();
	if (!bytes)
		return false;

	auto& buffer = d->output[d->readPtr];
	bool res = buffer.read(bytes);

	// 跳到下一段
	move(d->readPtr, d->bufferNum);
	return res;
}

void GMMAudioFile_Stream::nextChunk(gm::GMlong chunkNum)
{
	D(d);
	d->chunkNum += chunkNum;
	if (chunkNum > 0)
		d->blockWriteEvent.set();
}

void GMMAudioFile_Stream::waitForStreamReady()
{
	D(d);
	d->streamReadyEvent.wait();
}

void GMMAudioFile_Stream::rewindDecode()
{
	D(d);
	// 重启解码线程
	d->writePtr = 0;
	d->readPtr = 0;
	nextChunk(d->bufferNum);
	startDecodeThread();
}

void GMMAudioFile_Stream::rewind()
{
	cleanUp();
	rewindDecode();
}

void GMMAudioFile_Stream::cleanUp()
{
	D(d);
	d->blockWriteEvent.set();

	for (gm::GMuint i = 0; i < d->bufferNum; ++i)
	{
		if (d->output[i].isWriting())
		{
			d->output[i].endWrite(); //让数据读完，防止死锁
			break; //不可能同时写多个buffer
		}
	}
}

void GMMAudioFile_Stream::saveBuffer(Data* d, gm::GMbyte data)
{
	d->output[d->writePtr].beginWrite();
	d->output[d->writePtr] << data;
	if (d->output[d->writePtr].isFull())
	{
		d->output[d->writePtr].endWrite();
		--d->chunkNum;
		GM_ASSERT(d->chunkNum >= 0);

		// 跳到下一段缓存
		move(d->writePtr, d->bufferNum);
		d->output[d->writePtr].beginWrite();
		d->output[d->writePtr].rewind();

		if (d->chunkNum == 0)
		{
			// 如果所以缓存写满，等待
			d->blockWriteEvent.reset();
			d->blockWriteEvent.wait();
		}
	}
}

void GMMAudioFile_Stream::move(std::atomic_long& ptr, gm::GMuint loop)
{
	++ptr;
	ptr = ptr % loop;
}

void GMMAudioFile_Stream::setStreamReady(Data* d)
{
	d->streamReadyEvent.set();
}
