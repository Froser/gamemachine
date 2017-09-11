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
	ALuint bufferId = 0;
	ALuint sourceId = 0;
};

class GMMAudioStaticSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioStaticSource)

public:
	GMMAudioStaticSource(ALuint bufferId, ALuint sourceId);
	~GMMAudioStaticSource();

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;

private:
	void operate(GMMAudioSourcePlayOperation op);
};

GMMAudioStaticSource::GMMAudioStaticSource(ALuint bufferId, ALuint sourceId)
{
	D(d);
	d->bufferId = bufferId;
	d->sourceId = sourceId;
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
	ALuint bufferNum = 0;
	ALuint* bufferIds = nullptr;
	ALuint sourceId = 0;
	gm::IAudioFile* file;
};

class GMMAudioStreamSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioStreamSource)

public:
	GMMAudioStreamSource(ALuint bufferNum, ALuint* buffers, ALuint sourceId, gm::IAudioFile* file);
	~GMMAudioStreamSource();

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;
};

GMMAudioStreamSource::GMMAudioStreamSource(ALuint bufferNum, ALuint* buffers, ALuint sourceId, gm::IAudioFile* file)
{
	D(d);
	d->bufferNum = bufferNum;
	d->bufferIds = new ALuint[d->bufferNum]{ 0 };
	d->sourceId = sourceId;
	d->file = file;
}

GMMAudioStreamSource::~GMMAudioStreamSource()
{
	D(d);
	GM_ASSERT(d->bufferIds);
	delete[] d->bufferIds;
	alDeleteSources(1, &d->sourceId);
}

void GMMAudioStreamSource::play(bool loop)
{
	D(d);
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
		ALuint uiBufferID = 0;
		GMM_CHECK_AL_ERROR();
		alGenBuffers(1, &uiBufferID);
		GMM_CHECK_AL_ERROR();

		const gm::GMAudioFileInfo& fileInfo = f->getFileInfo();
		GMM_CHECK_AL_ERROR();
		alBufferData(uiBufferID, fileInfo.format, fileInfo.data, fileInfo.size, fileInfo.frequency);
		GMM_CHECK_AL_ERROR();
		f->disposeAudioFile();

		ALuint uiSource = 0;
		alGenSources(1, &uiSource);
		alSourcei(uiSource, AL_BUFFER, uiBufferID);

		GMMAudioStaticSource* h = new GMMAudioStaticSource(uiBufferID, uiSource);
		(*handle) = h;
	}
	else
	{
		ALuint uiBuffers[STREAM_BUFFER_NUM] = { 0 };
		alGenBuffers(STREAM_BUFFER_NUM, uiBuffers);
		ALuint uiSource = 0;
		alGenSources(1, &uiSource);

		const gm::GMAudioFileInfo& fileInfo = f->getFileInfo();
		gm::GMulong ulBufferSize = fileInfo.waveFormatExHeader.nAvgBytesPerSec >> 2;

		// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment
		ulBufferSize -= (ulBufferSize % fileInfo.waveFormatExHeader.nBlockAlign);

		GMMAudioStreamSource* h = new GMMAudioStreamSource(STREAM_BUFFER_NUM, uiBuffers, uiSource, f);
		(*handle) = h;
	}
}