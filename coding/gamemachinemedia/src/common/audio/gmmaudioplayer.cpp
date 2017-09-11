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
};

GM_PRIVATE_OBJECT(GMAudioHandlePlayThread)
{
	ALint sourceId = 0;
	GMAudioSourcePlaySignal signal = GMAudioSourcePlaySignal::None;
	bool loop = false;
	bool threadRunning = false;
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
		setSignal(signal);
		if (!d->threadRunning)
		{
			if (d->signal == GMAudioSourcePlaySignal::Play)
				this->start();
		}
	}

	ALint getSourceState()
	{
		D(d);
		ALint state;
		alGetSourcei(d->sourceId, AL_SOURCE_STATE, &state);
		return state;
	}

private:
	virtual void run() override
	{
		D(d);
		d->threadRunning = true;
		ALint state = AL_INVALID_VALUE;
		do
		{
			Sleep(1000 / 60); // 1 frame

			switch (d->signal)
			{
			case GMAudioSourcePlaySignal::Stop:
				alSourceStop(d->sourceId);
				break;
			case GMAudioSourcePlaySignal::Pause:
				alSourcePause(d->sourceId);
				break;
			case GMAudioSourcePlaySignal::Rewind:
				alSourceRewind(d->sourceId);
				break;
			case GMAudioSourcePlaySignal::Play:
				alSourcePlay(d->sourceId);
				break;
			case GMAudioSourcePlaySignal::None:
			default:
				break;
			}

			alGetSourcei(d->sourceId, AL_SOURCE_STATE, &state);
			if (d->signal == GMAudioSourcePlaySignal::Stop ||
				d->signal == GMAudioSourcePlaySignal::Pause ||
				d->signal == GMAudioSourcePlaySignal::Rewind )
			{
				break;
			}

			if (state != AL_PLAYING)
			{
				if (d->loop && d->signal == GMAudioSourcePlaySignal::None)
				{
					alSourceRewind(d->sourceId);
					alSourcePlay(d->sourceId);
				}
				else
				{
					break;
				}
			}

			setSignal(GMAudioSourcePlaySignal::None);

		} while (1);
		d->threadRunning = false;
	}

private:
	void setSignal(GMAudioSourcePlaySignal signal)
	{
		D(d);
		gm::GMMutex m;
		d->signal = signal;
	}
};

GM_PRIVATE_OBJECT(GMMAudioSource)
{
	ALuint bufferId = 0;
	ALuint sourceId = 0;
	GMAudioHandlePlayThread playThread;
};

class GMMAudioSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioSource)

public:
	GMMAudioSource(ALuint bufferId, ALuint sourceId);
	~GMMAudioSource();

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

GMMAudioSource::~GMMAudioSource()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Stop);
	d->playThread.join();
	alDeleteSources(1, &d->sourceId);
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
	d->playThread.join();
}

void GMMAudioSource::pause()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Pause);
	d->playThread.join();
}

void GMMAudioSource::rewind()
{
	D(d);
	d->playThread.emitSignal(GMAudioSourcePlaySignal::Rewind);
	d->playThread.join();
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