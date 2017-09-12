#include "stdafx.h"
#include "gmmaudioplayer.h"
#include "al.h"
#include "alc.h"
#include "decoder.h"

// 大小保持一致
static_assert(sizeof(gm::GMint) == sizeof(ALenum), "Size error");
static_assert(sizeof(gm::GMint) == sizeof(ALsizei), "Size error");
static ALenum s_alErrCode;
#define GMM_CHECK_AL_ERROR() GM_ASSERT((s_alErrCode = alGetError()) == AL_NO_ERROR);
//////////////////////////////////////////////////////////////////////////
enum class GMMAudioSourcePlayOperation
{
	None,
	Pause,
	Stop,
	Play,
	Rewind,
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMMAudioStaticSource)
{
	gm::IAudioFile* file = nullptr;
	ALuint sourceId = 0;
};

class GMMAudioStaticSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioStaticSource)

public:
	GMMAudioStaticSource(gm::IAudioFile* f);
	~GMMAudioStaticSource();

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;

private:
	void operate(GMMAudioSourcePlayOperation op);
};

GMMAudioStaticSource::GMMAudioStaticSource(gm::IAudioFile* f)
{
	D(d);
	d->file = f;
	alGenSources(1, &d->sourceId);
	alSourcei(d->sourceId, AL_BUFFER, d->file->getBufferId());
}

GMMAudioStaticSource::~GMMAudioStaticSource()
{
	D(d);
	operate(GMMAudioSourcePlayOperation::Stop);
	alDeleteSources(1, &d->sourceId);
}

void GMMAudioStaticSource::play(bool loop)
{
	D(d);
	alSourcei(d->sourceId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	operate(GMMAudioSourcePlayOperation::Play);
}

void GMMAudioStaticSource::stop()
{
	D(d);
	operate(GMMAudioSourcePlayOperation::Stop);
}

void GMMAudioStaticSource::pause()
{
	D(d);
	operate(GMMAudioSourcePlayOperation::Pause);
}

void GMMAudioStaticSource::rewind()
{
	D(d);
	operate(GMMAudioSourcePlayOperation::Rewind);
}

void GMMAudioStaticSource::operate(GMMAudioSourcePlayOperation op)
{
	D(d);
	switch (op)
	{
	case GMMAudioSourcePlayOperation::Stop:
		alSourceStop(d->sourceId);
		break;
	case GMMAudioSourcePlayOperation::Pause:
		alSourcePause(d->sourceId);
		break;
	case GMMAudioSourcePlayOperation::Rewind:
		alSourceRewind(d->sourceId);
		break;
	case GMMAudioSourcePlayOperation::Play:
		alSourcePlay(d->sourceId);
		break;
	case GMMAudioSourcePlayOperation::None:
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMMAudioStreamSource)
{
	ALuint* buffers = nullptr;
	ALuint bufferNum = 0;
	ALuint bufferSize = 0;
	ALuint sourceId = 0;
	gm::IAudioFile* file;
};

class GMMAudioStreamSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioStreamSource)

public:
	GMMAudioStreamSource(gm::IAudioFile* file);
	~GMMAudioStreamSource();

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;
};

GMMAudioStreamSource::GMMAudioStreamSource(gm::IAudioFile* file)
{
	D(d);
	d->file = file;
	gm::IAudioStream* stream = d->file->getStream();
	d->buffers = new ALuint[stream->getBufferNum()];
	alGenBuffers(stream->getBufferNum(), d->buffers);
	alGenSources(1, &d->sourceId);
}

GMMAudioStreamSource::~GMMAudioStreamSource()
{
	D(d);
	alDeleteSources(1, &d->sourceId);

	if (d->buffers)
	{
		gm::IAudioStream* stream = d->file->getStream();
		alDeleteBuffers(stream->getBufferNum(), d->buffers);
		delete[] d->buffers;
	}
}

void GMMAudioStreamSource::play(bool loop)
{
	D(d);
	GM_ASSERT(d->file->isStream());
	gm::IAudioStream* stream = d->file->getStream();
	gm::GMuint bufferNum = stream->getBufferNum();

	// 初始化
	stream->readBuffer(0, nullptr);
	gm::GMuint bufferSize = stream->getBufferSize();
	gm::GMbyte* audioData = new gm::GMbyte[bufferSize];

	// 先填充Buffer
	auto& fileInfo = d->file->getFileInfo();
	for (gm::GMuint i = 0; i < bufferNum; ++i)
	{
		stream->readBuffer(i, audioData);
		alBufferData(d->buffers[i], fileInfo.format, audioData, bufferSize, fileInfo.frequency);
		alSourceQueueBuffers(d->sourceId, 1, &d->buffers[i]);
	}
	alSourcePlay(d->sourceId);
}

void GMMAudioStreamSource::stop()
{

}

void GMMAudioStreamSource::pause()
{

}

void GMMAudioStreamSource::rewind()
{

}

//////////////////////////////////////////////////////////////////////////
GMMAudioPlayer::GMMAudioPlayer()
{
	openDevice(0);
}

GMMAudioPlayer::~GMMAudioPlayer()
{
	shutdownDevice();
}

bool GMMAudioPlayer::openDevice(gm::GMint idx)
{
	D(d);
	bool bRet = false;
	ALCdevice* device = nullptr;
	ALCcontext* context = nullptr;
	if ((device = alcOpenDevice(d->devices.GetDeviceName(idx))))
	{
		context = alcCreateContext(device, NULL);
		if (context)
		{
			gm_info("\nOpened %s Device\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
			alcMakeContextCurrent(context);
			bRet = AL_TRUE;
		}
		else
		{
			alcCloseDevice(device);
		}
	}
	return bRet;
}

void GMMAudioPlayer::shutdownDevice()
{
	ALCcontext* pContext = alcGetCurrentContext();
	ALCdevice* pDevice = alcGetContextsDevice(pContext);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(pContext);
	alcCloseDevice(pDevice);
}

void GMMAudioPlayer::createPlayerSource(gm::IAudioFile* f, OUT gm::IAudioSource** handle)
{
	if (!f->isStream())
	{
		GMMAudioStaticSource* h = new GMMAudioStaticSource(f);
		(*handle) = h;
	}
	else
	{
		GMMAudioStreamSource* h = new GMMAudioStreamSource(f);
		(*handle) = h;
	}
}