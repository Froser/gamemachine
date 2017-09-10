#include "stdafx.h"
#include "gmmaudioplayer.h"
#include "al.h"
#include "alc.h"

// 大小保持一致
static_assert(sizeof(gm::GMint) == sizeof(ALenum), "Size error");
static_assert(sizeof(gm::GMint) == sizeof(ALsizei), "Size error");
static ALenum s_alErrCode;
#define GMM_CHECK_AL_ERROR() GM_ASSERT((s_alErrCode = alGetError()) == AL_NO_ERROR);
//////////////////////////////////////////////////////////////////////////
enum class GMAudioSourcePlaySignal
{
	None,
	Pause,
	Stop,
	Play,
	Rewind,
	Terminate,
};

GM_PRIVATE_OBJECT(GMAudioHandlePlayThread)
{
	ALint sourceId = 0;
	GMAudioSourcePlaySignal signal = GMAudioSourcePlaySignal::None;
	bool loop = false;
};

class GMAudioHandlePlayThread : public gm::GMThread
{
	DECLARE_PRIVATE(GMAudioHandlePlayThread)

public:
	void setSource(ALint sourceId)
	{
		D(d);
		d->sourceId = sourceId;
	}

	void setLoop(bool loop)
	{
		D(d);
		d->loop = loop;
	}

	void emitSignal(GMAudioSourcePlaySignal signal)
	{
		D(d);
		d->signal = signal;
		if (d->signal == GMAudioSourcePlaySignal::Play)
			this->start();
	}

	ALint getSourceState()
	{
		D(d);
		ALint state;
		alGetSourcei(d->sourceId, AL_SOURCE_STATE, &state);
		return state;
	}

	void playSource()
	{
		D(d);
		alSourcePlay(d->sourceId);
	}

public:
	virtual void run() override
	{
		D(d);
		ALint state = AL_INVALID_VALUE;
		bool breakLoop = false;
		while (d->loop && !breakLoop)
		{
			do
			{
				Sleep(1000 / 60);

				switch (d->signal)
				{
				case GMAudioSourcePlaySignal::Stop:
					alSourceStop(d->sourceId);
					resetSignal();
					breakLoop = true;
					break;
				case GMAudioSourcePlaySignal::Pause:
					alSourcePause(d->sourceId);
					resetSignal();
					breakLoop = true;
					break;
				case GMAudioSourcePlaySignal::Rewind:
					alSourceRewind(d->sourceId);
					resetSignal();
					break;
				case GMAudioSourcePlaySignal::Play:
					playSource();
					resetSignal();
					break;
				case GMAudioSourcePlaySignal::None:
				default:
					break;
				}

				if (breakLoop)
					break;

				alGetSourcei(d->sourceId, AL_SOURCE_STATE, &state);
			} while (state == AL_PLAYING);

			if (d->loop && !breakLoop)
				playSource();
		}
		resetSignal();
	}

private:
	void resetSignal()
	{
		D(d);
		d->signal = GMAudioSourcePlaySignal::None;
	}
};

GM_PRIVATE_OBJECT(GMMAudioSource)
{
	ALint bufferId;
	ALint sourceId;
	GMAudioHandlePlayThread playThread;
};

class GMMAudioSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioSource)

public:
	GMMAudioSource(ALuint bufferId, ALuint sourceId);

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;
};

GMMAudioSource::GMMAudioSource(ALuint bufferId, ALuint sourceId)
{
	D(d);
	d->bufferId = bufferId;
	d->sourceId = sourceId;
}

void GMMAudioSource::play(bool loop)
{
	D(d);
	d->playThread.setSource(d->sourceId);
	d->playThread.setLoop(loop);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Play);
}

void GMMAudioSource::stop()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Stop);
}

void GMMAudioSource::pause()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Pause);
}

void GMMAudioSource::rewind()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Rewind);
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
	ALuint uiBufferID = -1;
	GMM_CHECK_AL_ERROR();
	alGenBuffers(1, &uiBufferID);
	GMM_CHECK_AL_ERROR();

	const gm::GMAudioFileInfo& fileInfo = f->getFileInfo();
	GMM_CHECK_AL_ERROR();
	alBufferData(uiBufferID, fileInfo.format, fileInfo.data, fileInfo.size, fileInfo.frequency);
	GMM_CHECK_AL_ERROR();
	f->disposeAudioFile();

	ALuint uiSource = -1;
	alGenSources(1, &uiSource);
	alSourcei(uiSource, AL_BUFFER, uiBufferID);

	GMMAudioSource* h = new GMMAudioSource(uiBufferID, uiSource);
	(*handle) = h;
}