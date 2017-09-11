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
	gm::GMAudioFileInfo fileInfo;
	bool fmtCreated = false;
	Vector<gm::GMbyte> output;
	GMMDecodeThread decodeThread;
};

class GMMAudioFile_MP3 : public gm::IAudioFile
{
	DECLARE_PRIVATE(GMMAudioFile_MP3)

public:
	GMMAudioFile_MP3()
	{
	}

	~GMMAudioFile_MP3()
	{
		disposeAudioFile();
	}

public:
	bool load(gm::GMBuffer& buffer)
	{
		D(d);
		d->fileInfo.data = buffer.buffer;
		d->fileInfo.size = buffer.size;
		d->decodeThread.setData(d, decode);
		d->decodeThread.start();
		return true;
	}

public:
	virtual bool isStream() override
	{
		return true;
	}

	virtual void disposeAudioFile() override
	{
	}

	virtual gm::GMAudioFileInfo& getFileInfo() override
	{
		D(d);
		return d->fileInfo;
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
		mad_stream_buffer(stream, (unsigned char*)d->fileInfo.data, d->fileInfo.size);
		return MAD_FLOW_CONTINUE;
	}

	static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
	{
		Data* d = (Data*)data;
		createFormat(d, pcm);

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

			d->output.push_back((sample >> 0) & 0xff);
			d->output.push_back((sample >> 8) & 0xff);

			if (nchannels == 2) {
				sample = scale(*right_ch++);
				d->output.push_back((sample >> 0) & 0xff);
				d->output.push_back((sample >> 8) & 0xff);
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

	static void createFormat(Data* d, struct mad_pcm *pcm)
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
			d->fmtCreated = true;
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
