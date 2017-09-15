#include "stdafx.h"
#include "gmmaudioreader_stream.h"
#include "common/utilities/gmmstream.h"

GMMAudioFile_Stream::GMMAudioFile_Stream()
{
	init();
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
	d->streamReadyEvent.reset();
	nextChunk(d->bufferNum - 1);
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

	GM_ASSERT(bytes);
	auto& buffer = d->output[d->readPtr];
	buffer.read(bytes);

	// 跳到下一段
	move(d->readPtr, d->bufferNum);
	return d->eof && peek(d->readPtr, d->bufferNum) == d->eofPtr;
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
	init();
	nextChunk(d->bufferNum - 1);
	startDecodeThread();
}

void GMMAudioFile_Stream::rewind()
{
	cleanUp();
	rewindDecode();
}

void GMMAudioFile_Stream::init()
{
	D(d);
	d->writePtr = 0;
	d->readPtr = 0;
	d->chunkNum = 0;
	d->eof = false;
}

void GMMAudioFile_Stream::cleanUp()
{
	D(d);
	d->blockWriteEvent.set();

	for (gm::GMuint i = 0; i < d->bufferNum; ++i)
	{
		if (d->output[i].isWriting())
			d->output[i].endWrite(); //让数据读完，防止死锁
		d->output[i].rewind();
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
		d->output[d->writePtr].rewind();
		d->output[d->writePtr].beginWrite();

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

gm::GMlong GMMAudioFile_Stream::peek(std::atomic_long& ptr, gm::GMuint loop)
{
	return (ptr + 1) % loop;
}

void GMMAudioFile_Stream::setEof(Data* d)
{
	// 做个EOF标记
	// 流可能是循环的，这个标记表示在这个buffer中，到达了解码的末尾
	d->output[d->writePtr].fill(0x00);
	d->eof = true;
	d->eofPtr = d->writePtr;
}

void GMMAudioFile_Stream::setStreamReady(Data* d)
{
	d->streamReadyEvent.set();
}
