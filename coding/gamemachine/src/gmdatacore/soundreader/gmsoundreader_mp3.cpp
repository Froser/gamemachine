#include "stdafx.h"
#include "gmsoundreader_mp3.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "mad.h"
#if _WINDOWS
#include <dsound.h>
#endif
#include "foundation/utilities/utilities.h"
#include "os/gmdirectsound_sounddevice.h"

#define FRAME_BUFFER 50 // 每一份缓存包含的帧数
#define BUFFER_COUNT 2 // 将缓存分为多少部分

class MP3SoundFile;
GM_PRIVATE_OBJECT(MP3SoundFile)
{
	MP3SoundFile* parent;
	bool inited; //是否已经初始化
	bool formatCreated; // 是否已经生成了WAVEFORMATEX
	bool bufferLoaded; // 是否已经读取了所有MP3文件数据
	WAVEFORMATEX format;
	GMBuffer bufferIn; // MP3文件数据
	AlignedVector<GMbyte> bufferOut; // 缓存的字节
	GMuint bufferOffset; // 缓存偏移
	GMuint unitBufferSize; // 单位缓存，每次读取这么多大小的缓存
	bool playing; // 是否正在播放
	GMlong frame; // MP3已经解码的帧数

#if _WINDOWS
	DSBPOSITIONNOTIFY notifyPos[BUFFER_COUNT];
	HANDLE events[BUFFER_COUNT];
	ComPtr<IDirectSoundBuffer8> cpDirectSoundBuffer;
	HANDLE thread;
#endif
};

#if _WINDOWS
static DWORD WINAPI decode(LPVOID lpThreadParameter);
static DWORD WINAPI processBuffer(LPVOID lpThreadParameter);

class MP3SoundFile : public GMSoundFile
{
	DECLARE_PRIVATE(MP3SoundFile)

	typedef GMSoundFile Base;

public:
	MP3SoundFile(GMBuffer* in)
		: Base(WAVEFORMATEX(), nullptr)
	{
		D(d);
		d->parent = this;
		d->inited = false;
		d->bufferIn = *in;
		d->playing = false;
		d->formatCreated = false;
		d->bufferLoaded = false;
		d->frame = 0;
		d->bufferOffset = 0;
	}

	~MP3SoundFile()
	{
	}

public:
	virtual void play() override
	{
		D(d);
		d->playing = true;
		d->thread = ::CreateThread(NULL, NULL, ::decode, &d, NULL, NULL);
	}

	virtual void stop() override
	{
		D(d);
		d->playing = false;
		if (d->cpDirectSoundBuffer)
			d->cpDirectSoundBuffer->Stop();

		for (GMint i = 0; i < BUFFER_COUNT; i++)
		{
			::SetEvent(d->events[i]);
		}
	}

public:
	// 将缓冲填满
	void transfer(GMbyte* b, GMuint size)
	{
		D(d);
		if (!d->inited)
		{
			init(b, size);
			d->inited = true;
			::CreateThread(NULL, NULL, processBuffer, &d, NULL, NULL);
		}
	}

private:
	void init(GMbyte* b, GMuint size)
	{
		D(d);
		DSBUFFERDESC dsbd = { 0 };
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
		dsbd.dwBufferBytes = size;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX)&d->format;
		d->unitBufferSize = size / BUFFER_COUNT;

		ComPtr<IDirectSoundBuffer> cpBuffer;
		HRESULT hr;
		if (FAILED(hr = GMSoundPlayerDevice::getInstance()->CreateSoundBuffer(&dsbd, &cpBuffer, NULL)))
		{
			gm_error(_L("create sound buffer error."));
			return;
		}

		if (FAILED(hr = cpBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&d->cpDirectSoundBuffer)))
		{
			gm_error(_L("QueryInterface to IDirectSoundBuffer8 error"));
			return;
		}

		ComPtr<IDirectSoundNotify> cpDirectSoundNotify;
		if (FAILED(hr = d->cpDirectSoundBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&cpDirectSoundNotify)))
		{
			gm_error(_L("QueryInterface to IDirectSoundNotify error"));
			return;
		}

		for (GMint i = 0; i < BUFFER_COUNT; i++)
		{
			d->notifyPos[i].dwOffset = (i + 1) * d->unitBufferSize - 1;
			d->notifyPos[i].hEventNotify = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			d->events[i] = d->notifyPos[i].hEventNotify;
		}
		hr = cpDirectSoundNotify->SetNotificationPositions(2, d->notifyPos);
		ASSERT(SUCCEEDED(hr));

		LPVOID lpLockBuf;
		DWORD len;
		hr = d->cpDirectSoundBuffer->Lock(0, 0, &lpLockBuf, &len, 0, 0, DSBLOCK_ENTIREBUFFER);
		ASSERT(SUCCEEDED(hr));
		memcpy(lpLockBuf, b, size);
		memset((GMbyte*)lpLockBuf + size, 0, len - size);
		hr = d->cpDirectSoundBuffer->Unlock(lpLockBuf, len, NULL, NULL);
		ASSERT(SUCCEEDED(hr));
		hr = d->cpDirectSoundBuffer->SetCurrentPosition(0);
		ASSERT(SUCCEEDED(hr));
		hr = d->cpDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		ASSERT(SUCCEEDED(hr));
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
#if _WINDOWS
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)data;

	if (!d->playing)
		return MAD_FLOW_STOP;

	if (d->bufferLoaded)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, d->bufferIn.buffer, d->bufferIn.size);
	d->bufferLoaded = true;
	return MAD_FLOW_CONTINUE;
#else
	return MAD_FLOW_CONTINUE;
#endif
}

static mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
#if _WINDOWS
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)data;

	if (!d->playing)
		return MAD_FLOW_STOP;

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

	if (d->frame == FRAME_BUFFER * BUFFER_COUNT)
	{
		// 先读取BUFFER_COUNT倍的数据到内存
		GMint transfered = d->bufferOut.size();
		d->parent->transfer(d->bufferOut.data() + d->bufferOffset, transfered);
		d->bufferOffset += transfered;
	}
	if (d->frame < (FRAME_BUFFER * BUFFER_COUNT) + 1)
		d->frame++;

	return MAD_FLOW_CONTINUE;
#else
	return MAD_FLOW_CONTINUE;
#endif
}


#if _WINDOWS
static DWORD WINAPI decode(LPVOID lpThreadParameter)
{
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)lpThreadParameter;
	mad_decoder decoder;
	mad_decoder_init(&decoder, d, input, nullptr, nullptr, output, nullptr, nullptr);
	GMint result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	return mad_decoder_finish(&decoder) == 0;
}

static DWORD WINAPI processBuffer(LPVOID lpThreadParameter)
{
	MP3SoundFile::Data* d = (MP3SoundFile::Data*)lpThreadParameter;
	DWORD bufferOffset = 0;

	// 手动增加个计数，防止调用到一半被析构
	ComPtr<IDirectSoundBuffer8> dummy = d->cpDirectSoundBuffer;

	while (d->playing)
	{
		WaitForMultipleObjects(BUFFER_COUNT, d->events, FALSE, INFINITE);

		LPVOID lpvPtr1;
		DWORD dwBytes1;
		LPVOID lpvPtr2;
		DWORD dwBytes2;
		HRESULT hr;
		hr = d->cpDirectSoundBuffer->Lock(bufferOffset, d->unitBufferSize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
		// If the buffer was lost, restore and retry lock. 
		if (DSERR_BUFFERLOST == hr)
		{
			d->cpDirectSoundBuffer->Restore();
			hr = d->cpDirectSoundBuffer->Lock(bufferOffset, d->unitBufferSize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
		}

		if (SUCCEEDED(hr))
		{
			memcpy(lpvPtr1, d->bufferOut.data() + d->bufferOffset, dwBytes1);
			if (lpvPtr2)
				memcpy(lpvPtr2, d->bufferOut.data() + d->bufferOffset + dwBytes1, dwBytes2);

			// Release the data back to DirectSound. 
			hr = d->cpDirectSoundBuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
			ASSERT(SUCCEEDED(hr));
		}

		DWORD step = dwBytes1 + dwBytes2;
		d->bufferOffset += step;
		bufferOffset += step;
		bufferOffset %= d->unitBufferSize * BUFFER_COUNT;

		if (d->bufferOffset > d->bufferOut.size())
		{
			//TODO: do not loop
			break;
		}
	}

	for (GMint i = 0; i < BUFFER_COUNT; i++)
	{
		::CloseHandle(d->events[i]);
	}
	return 0;
}
#endif

bool GMSoundReader_MP3::load(GMBuffer& buffer, OUT ISoundFile** sf)
{
#if _WINDOWS
	D(d);
	MP3SoundFile* file = new MP3SoundFile(&buffer);
	*sf = file;
	return 0;
#else
	ASSERT(false);
	return 0;
#endif
}

bool GMSoundReader_MP3::test(const GMBuffer& buffer)
{
	return buffer.buffer[0] == 'I' && buffer.buffer[1] == 'D' && buffer.buffer[2] == '3';
}
