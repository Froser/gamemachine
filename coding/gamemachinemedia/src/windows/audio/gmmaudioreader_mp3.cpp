#include "stdafx.h"
#include <gmcom.h>
#include "gmmaudioreader_mp3.h"
#include "alframework/cwaves.h"
#include <al.h>
#include "mad.h"

typedef void(*DecodeCallback)(void* data);

GM_PRIVATE_OBJECT(GMMDecodeThread)
{
	void* data;
	DecodeCallback callback;
	bool started = false;
};

class GMMDecodeThread : public gm::GMThread
{
	DECLARE_PRIVATE(GMMDecodeThread)

public:
	void setData(void* dt, DecodeCallback callback)
	{
		D(d);
		d->data = dt;
		d->callback = callback;
	}

private:
	virtual void run()
	{
		D(d);
		d->callback(d->data);
	}
};

GM_PRIVATE_OBJECT(GMMAudioFile_MP3)
{
	gm::GMuint bufferNum = 0;
	gm::GMAudioFileInfo fileInfo;
	bool fmtCreated = false;
	Vector<gm::GMbyte>* output = nullptr;
	GMMDecodeThread decodeThread;
	bool terminateDecode = false;
	gm::GMEvent terminateEvent;

	// stream相关
	gm::GMulong bufferSize = 0;
	gm::GMuint writePtr = 0;
	gm::GMuint readPtr = 0;
	gm::GMEvent bufferWriteEvent;
	gm::GMEvent bufferReadEvent;
};

class GMMAudioFile_MP3 : public gm::IAudioFile, public gm::IAudioStream
{
	DECLARE_PRIVATE(GMMAudioFile_MP3)

public:
	GMMAudioFile_MP3()
	{
	}

	~GMMAudioFile_MP3()
	{
		D(d);
		d->decodeThread.detach();
		d->terminateDecode = true;
		d->terminateEvent.wait();
	}

public:
	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		d->fileInfo.data = buffer.buffer;
		d->fileInfo.size = buffer.size;
		d->bufferNum = getBufferNum();
		d->decodeThread.setData(d, decode);
		d->decodeThread.start();
		return true;
	}

	// gm::IAudioFile
public:
	virtual bool isStream() override
	{
		return true;
	}

	virtual gm::IAudioStream* getStream() override
	{
		return this;
	}

	virtual gm::GMAudioFileInfo& getFileInfo() override
	{
		D(d);
		return d->fileInfo;
	}

	virtual gm::GMuint getBufferId() override
	{
		return 0;
	}

	// gm::IAudioStream
public:
	virtual gm::GMuint getBufferNum() override
	{
		return STREAM_BUFFER_NUM;
	}

	virtual gm::GMuint getBufferSize() override
	{
		D(d);
		return d->bufferSize;
	}

	virtual bool readBuffer(gm::GMbyte* bytes) override
	{
		D(d);
		d->bufferReadEvent.wait();
		d->bufferReadEvent.set();
		if (!bytes)
			return false;

		auto& buffer = d->output[d->readPtr];
		gm::GMuint sz = buffer.size();
		if (!sz)
			return false;

		memcpy_s(bytes, sz, buffer.data(), sz);

		++d->readPtr;
		d->readPtr = d->readPtr % d->bufferNum;
		return true;
	}

	virtual void nextBuffer() override
	{
		D(d);
		d->bufferWriteEvent.set();
	}
	
private: // MP3解码器
	static void decode(void* data)
	{
		Data* d = (Data*)data;
		d->terminateEvent.reset();
		mad_decoder decoder;
		mad_decoder_init(&decoder, d, input, nullptr, nullptr, output, nullptr, nullptr);
		mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
		mad_decoder_finish(&decoder);
		d->terminateEvent.set();
	}

	static mad_flow input(void *data, mad_stream *stream)
	{
		Data* d = (Data*)data;
		mad_stream_buffer(stream, (unsigned char*)d->fileInfo.data, d->fileInfo.size);
		return MAD_FLOW_CONTINUE;
	}

	static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
	{
		Data* d = (Data*)data;
		init(d, pcm);

		if (d->terminateDecode)
			return MAD_FLOW_STOP;

		gm::GMuint nchannels, nsamples;
		mad_fixed_t const *left_ch, *right_ch;

		/* pcm->samplerate contains the sampling frequency */
		nchannels = pcm->channels;
		nsamples = pcm->length;
		left_ch = pcm->samples[0];
		right_ch = pcm->samples[1];

		while (nsamples--) {
			signed int sample;

			/* output sample(s) in 16-bit signed little-endian PCM */

			sample = scale(*left_ch++);

			saveBuffer(d, (sample >> 0) & 0xff);
			saveBuffer(d, (sample >> 8) & 0xff);

			if (nchannels == 2) {
				sample = scale(*right_ch++);
				saveBuffer(d, (sample >> 0) & 0xff);
				saveBuffer(d, (sample >> 8) & 0xff);
			}
		}

		return MAD_FLOW_CONTINUE;
	}

	static inline gm::GMint scale(mad_fixed_t sample)
	{
		/* round */
		sample += (1L << (MAD_F_FRACBITS - 16));

		/* clip */
		if (sample >= MAD_F_ONE)
			sample = MAD_F_ONE - 1;
		else if (sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;

		/* quantize */
		return sample >> (MAD_F_FRACBITS + 1 - 16);
	}

	static void init(Data* d, struct mad_pcm *pcm)
	{
		if (!d->fmtCreated)
		{
			d->fileInfo.waveFormatExHeader.cbSize = sizeof(WAVEFORMATEX);
			d->fileInfo.waveFormatExHeader.nChannels = pcm->channels;
			d->fileInfo.waveFormatExHeader.nBlockAlign = 4;
			d->fileInfo.waveFormatExHeader.wFormatTag = 1;
			d->fileInfo.waveFormatExHeader.nSamplesPerSec = pcm->samplerate;
			d->fileInfo.waveFormatExHeader.nAvgBytesPerSec = d->fileInfo.waveFormatExHeader.nSamplesPerSec * sizeof(unsigned short)* pcm->channels;
			d->fileInfo.waveFormatExHeader.wBitsPerSample = 16;
			d->fileInfo.frequency = pcm->samplerate;
			d->fileInfo.format = getFileFormat(d, alGetEnumValue);
			d->fmtCreated = true;

			d->bufferSize = d->fileInfo.waveFormatExHeader.nAvgBytesPerSec >> 2;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment
			d->bufferSize -= (d->bufferSize % d->fileInfo.waveFormatExHeader.nBlockAlign);

			GM_ASSERT(!d->output);
			d->output = new Vector<gm::GMbyte>[d->bufferSize];
			for (gm::GMuint i = 0; i < d->bufferNum; i++)
			{
				d->output[i].reserve(d->bufferSize);
			}
		}
	}

	typedef int(__cdecl *PFNALGETENUMVALUE)(const char *szEnumName);

	static ALenum getFileFormat(Data* d, PFNALGETENUMVALUE pfnGetEnumValue)
	{
		if (d->fileInfo.waveFormatExHeader.nChannels == 1)
		{
			switch (d->fileInfo.waveFormatExHeader.wBitsPerSample)
			{
			case 4:
				return pfnGetEnumValue("AL_FORMAT_MONO_IMA4");
			case 8:
				return pfnGetEnumValue("AL_FORMAT_MONO8");
			case 16:
				return pfnGetEnumValue("AL_FORMAT_MONO16");
			}
		}
		else if (d->fileInfo.waveFormatExHeader.nChannels == 2)
		{
			switch (d->fileInfo.waveFormatExHeader.wBitsPerSample)
			{
			case 4:
				return pfnGetEnumValue("AL_FORMAT_STEREO_IMA4");
			case 8:
				return pfnGetEnumValue("AL_FORMAT_STEREO8");
			case 16:
				return pfnGetEnumValue("AL_FORMAT_STEREO16");
			}
		}
		else if (d->fileInfo.waveFormatExHeader.nChannels == 4 && (d->fileInfo.waveFormatExHeader.wBitsPerSample == 16))
			return pfnGetEnumValue("AL_FORMAT_QUAD16");
		GM_ASSERT(false);
		return AL_INVALID_ENUM;
	}

	static void saveBuffer(Data* d, gm::GMbyte data)
	{
		// 当读取指针和写入指针在同一段，锁定它，直到这一段读取完毕
		if (d->writePtr == d->readPtr)
			d->bufferReadEvent.reset();
		else
			d->bufferReadEvent.set();

		d->output[d->writePtr].push_back(data);
		if (d->output[d->writePtr].size() == d->bufferSize)
		{
			++d->writePtr;
			d->writePtr = d->writePtr % d->bufferNum;
			d->output[d->writePtr].clear();
			if (d->writePtr == d->readPtr)
			{
				// 缓存写满了，等待时机写入
				d->bufferWriteEvent.reset();
				d->bufferWriteEvent.wait();
			}
		}
	}
};

bool GMMAudioReader_MP3::load(gm::GMBuffer& buffer, OUT gm::IAudioFile** f)
{
	GMMAudioFile_MP3* file = new GMMAudioFile_MP3();
	(*f) = file;
	return file->load(buffer);
}

bool GMMAudioReader_MP3::test(const gm::GMBuffer& buffer)
{
	char header[3];
	gm::MemoryStream ms(buffer.buffer, buffer.size);
	ms.read((gm::GMbyte*)&header, sizeof(header));
	return strnEqual(header, "ID3", 3);
}
