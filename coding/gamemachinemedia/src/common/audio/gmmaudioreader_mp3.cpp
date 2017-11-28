#include "stdafx.h"
#include "gmmaudioreader_mp3.h"
#include <al/al.h>
#include "mad.h"
#include "common/utilities/gmmstream.h"
#include "gmmaudioreader_stream.h"

#define _MAD_CHECK_FLOW(i) if ((i) == MAD_FLOW_STOP) return MAD_FLOW_STOP;

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
	typedef GMMAudioFile_Stream Base;

	bool fmtCreated = false;
	GMMDecodeThread decodeThread;
	bool terminateDecode = false;
	Base::Data* baseData = nullptr;
};

class GMMAudioFile_MP3 : public GMMAudioFile_Stream
{
	DECLARE_PRIVATE(GMMAudioFile_MP3)

	typedef GMMAudioFile_Stream Base;

public:
	GMMAudioFile_MP3()
	{
		D(d);
		d->baseData = Base::data();
	}

	~GMMAudioFile_MP3()
	{
		D(d);
		d->decodeThread.detach();
		d->baseData->blockWriteEvent.set();
		d->terminateDecode = true;
		d->decodeThread.wait();
	}

public:
	virtual void startDecodeThread() override
	{
		D(d);
		d->decodeThread.setData(d, decode);
		d->decodeThread.start();
	}

	virtual void rewindDecode() override
	{
		D(d);
		// 清理解码线程
		d->decodeThread.detach();
		d->terminateDecode = true;
		d->decodeThread.wait();
		d->terminateDecode = false;

		Base::rewindDecode();
	}

private: // MP3解码器
	static void decode(void* data)
	{
		Data* d = (Data*)data;
		mad_decoder decoder;
		mad_decoder_init(&decoder, d, input, nullptr, nullptr, output, nullptr, nullptr);
		mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
		mad_decoder_finish(&decoder);
	}

	static mad_flow input(void *data, mad_stream *stream)
	{
		Data* d = (Data*)data;

		if (stream->error == MAD_ERROR_BUFLEN) //EOF
		{
			GMMAudioFile_Stream::setEof(d->baseData);
			mad_stream_buffer(stream, (unsigned char*)d->baseData->fileInfo.data, d->baseData->fileInfo.size);
			return MAD_FLOW_CONTINUE;
		}

		mad_stream_buffer(stream, (unsigned char*)d->baseData->fileInfo.data, d->baseData->fileInfo.size);
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

			_MAD_CHECK_FLOW(saveBufferIfNotTerminated(d, (sample >> 0) & 0xff));
			_MAD_CHECK_FLOW(saveBufferIfNotTerminated(d, (sample >> 8) & 0xff));

			if (nchannels == 2)
			{
				sample = scale(*right_ch++);
				_MAD_CHECK_FLOW(saveBufferIfNotTerminated(d, (sample >> 0) & 0xff));
				_MAD_CHECK_FLOW(saveBufferIfNotTerminated(d, (sample >> 8) & 0xff));
			}
		}

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow saveBufferIfNotTerminated(Data* d, gm::GMbyte byte)
	{
		if (!d->terminateDecode)
		{
			saveBuffer(d->baseData, byte);
			return MAD_FLOW_CONTINUE;
		}
		return MAD_FLOW_STOP;
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
			d->baseData->fileInfo.waveFormatExHeader.cbSize = sizeof(WAVEFORMATEX);
			d->baseData->fileInfo.waveFormatExHeader.nChannels = pcm->channels;
			d->baseData->fileInfo.waveFormatExHeader.nBlockAlign = 4;
			d->baseData->fileInfo.waveFormatExHeader.wFormatTag = 1;
			d->baseData->fileInfo.waveFormatExHeader.nSamplesPerSec = pcm->samplerate;
			d->baseData->fileInfo.waveFormatExHeader.nAvgBytesPerSec = d->baseData->fileInfo.waveFormatExHeader.nSamplesPerSec * sizeof(unsigned short)* pcm->channels;
			d->baseData->fileInfo.waveFormatExHeader.wBitsPerSample = 16;
			d->baseData->fileInfo.frequency = pcm->samplerate;
			d->baseData->fileInfo.format = getFileFormat(d, alGetEnumValue);
			d->fmtCreated = true;

			d->baseData->bufferSize = d->baseData->fileInfo.waveFormatExHeader.nAvgBytesPerSec >> 2;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment
			d->baseData->bufferSize -= (d->baseData->bufferSize % d->baseData->fileInfo.waveFormatExHeader.nBlockAlign);

			GM_ASSERT(!d->baseData->output);
			d->baseData->output = new GMMStream[d->baseData->bufferNum];
			for (gm::GMuint i = 0; i < d->baseData->bufferNum; i++)
			{
				d->baseData->output[i].resize(d->baseData->bufferSize);
			}

			// Stream is ready
			GMMAudioFile_Stream::setStreamReady(d->baseData);
		}
	}

	typedef int(__cdecl *PFNALGETENUMVALUE)(const char *szEnumName);

	static ALenum getFileFormat(Data* d, PFNALGETENUMVALUE pfnGetEnumValue)
	{
		if (d->baseData->fileInfo.waveFormatExHeader.nChannels == 1)
		{
			switch (d->baseData->fileInfo.waveFormatExHeader.wBitsPerSample)
			{
			case 4:
				return pfnGetEnumValue("AL_FORMAT_MONO_IMA4");
			case 8:
				return pfnGetEnumValue("AL_FORMAT_MONO8");
			case 16:
				return pfnGetEnumValue("AL_FORMAT_MONO16");
			}
		}
		else if (d->baseData->fileInfo.waveFormatExHeader.nChannels == 2)
		{
			switch (d->baseData->fileInfo.waveFormatExHeader.wBitsPerSample)
			{
			case 4:
				return pfnGetEnumValue("AL_FORMAT_STEREO_IMA4");
			case 8:
				return pfnGetEnumValue("AL_FORMAT_STEREO8");
			case 16:
				return pfnGetEnumValue("AL_FORMAT_STEREO16");
			}
		}
		else if (d->baseData->fileInfo.waveFormatExHeader.nChannels == 4 && (d->baseData->fileInfo.waveFormatExHeader.wBitsPerSample == 16))
			return pfnGetEnumValue("AL_FORMAT_QUAD16");
		GM_ASSERT(false);
		return AL_INVALID_ENUM;
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
	gm::GMMemoryStream ms(buffer.buffer, buffer.size);
	ms.read((gm::GMbyte*)&header, sizeof(header));
	return strnEqual(header, "ID3", 3);
}
