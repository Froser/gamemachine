#include "stdafx.h"
#include "soundreader_mp3.h"
#include "gmdatacore/gamepackage.h"
#include "mad.h"
#ifdef _WINDOWS
#include <dsound.h>
#endif
#include "utilities/comptr.h"
#include "os/directsound_sounddevice.h"

#define NOTIFYNUM 10
struct MP3SoundFilePrivate
{
	bool formatCreated;
	bool bufferLoaded;
	WAVEFORMATEX format;
	GamePackageBuffer* bufferIn;
	std::vector<GMbyte> bufferOut;

#ifdef _WINDOWS
	ComPtr<IDirectSoundBuffer8> cpDirectSoundBuffer;
	bool playing;

	ComPtr<IDirectSoundNotify> cpSoundNotify;
	DSBPOSITIONNOTIFY notifyPos[NOTIFYNUM];

	HANDLE thread;
#endif
};

#ifdef _WINDOWS
static DWORD WINAPI decode(LPVOID lpThreadParameter);

class MP3SoundFile : public SoundFile
{
	DEFINE_PRIVATE(MP3SoundFile)

	typedef SoundFile Base;

public:
	typedef MP3SoundFilePrivate Data;

public:
	MP3SoundFile(GamePackageBuffer* in)
		: Base(WAVEFORMATEX(), nullptr)
	{
		D(d);
		d.formatCreated = false;
		d.bufferLoaded = false;
		d.bufferIn = in;
		d.playing = false;
	}

public:
	virtual void play() override
	{
		D(d);
		d.thread = ::CreateThread(NULL, NULL, ::decode, &d, NULL, NULL);
	}

	virtual void stop() override
	{
	}

private:
	void loadSound()
	{
		D(d);
		DSBUFFERDESC dsbd = { 0 };
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
		dsbd.dwBufferBytes = getData()->dwSize;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX)getWaveFormat();

		ComPtr<IDirectSoundBuffer> cpBuffer;
		HRESULT hr;
		if (FAILED(hr = SoundPlayerDevice::getInstance()->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
		{
			gm_error("create sound buffer error.");
			return;
		}

		if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&d.cpDirectSoundBuffer)))
		{
			gm_error("QueryInterface to IDirectSoundBuffer8 error");
			return;
		}

		d.cpDirectSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&d.cpSoundNotify);
		for (GMint i = 0; i < NOTIFYNUM; i++)
		{
			d.notifyPos[i].dwOffset = 0;
			d.notifyPos[i].hEventNotify = 0;
		}
		d.cpSoundNotify->SetNotificationPositions(NOTIFYNUM, d.notifyPos);
	}
};
#endif

static inline GMint scale(mad_fixed_t sample)
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

static mad_flow input(void *data, mad_stream *stream)
{
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)data;

	if (d->bufferLoaded)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, d->bufferIn->buffer, d->bufferIn->size);
	d->bufferLoaded = true;
	return MAD_FLOW_CONTINUE;
}

static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)data;
	if (!d->formatCreated)
	{
		d->format.cbSize = sizeof(WAVEFORMATEX);
		d->format.nChannels = pcm->channels;
		d->format.nBlockAlign = 4;
		d->format.wFormatTag = 1;
		d->format.nSamplesPerSec = pcm->samplerate;
		d->format.nAvgBytesPerSec = d->format.nSamplesPerSec * sizeof(unsigned short)* pcm->channels;
		d->format.wBitsPerSample = 16;
		d->formatCreated = true;
	}

	GMuint nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	d->bufferOut.reserve(d->bufferOut.size() + (nchannels == 2 ? nsamples * 4 : nsamples * 2));

	while (nsamples--) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch++);
		d->bufferOut.push_back((sample >> 0) & 0xff);
		d->bufferOut.push_back((sample >> 8) & 0xff);

		if (nchannels == 2) {
			sample = scale(*right_ch++);
			d->bufferOut.push_back((sample >> 0) & 0xff);
			d->bufferOut.push_back((sample >> 8) & 0xff);
		}
	}

	return MAD_FLOW_CONTINUE;
}

static DWORD WINAPI decode(LPVOID lpThreadParameter)
{
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)lpThreadParameter;
	mad_decoder decoder;
	mad_decoder_init(&decoder, d, input, nullptr, nullptr, output, nullptr, nullptr);
	GMint result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	return mad_decoder_finish(&decoder) == 0;
}

bool SoundReader_MP3::load(GamePackageBuffer& buffer, OUT ISoundFile** sf)
{
	D(d);
	MP3SoundFile* file = new MP3SoundFile(&buffer);
	*sf = file;
	return 0;
}

bool SoundReader_MP3::test(const GamePackageBuffer& buffer)
{
	return buffer.buffer[0] == 'I' && buffer.buffer[1] == 'D' && buffer.buffer[2] == '3';
}