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

class GMMAudioStreamPlayThread;
GM_PRIVATE_OBJECT(GMMAudioStreamSource)
{
	ALuint* buffers = nullptr;
	ALuint bufferNum = 0;
	ALuint bufferSize = 0;
	ALuint sourceId = 0;
	gm::IAudioFile* file;
	GMMAudioStreamPlayThread* thread = nullptr;
};

GM_PRIVATE_OBJECT(GMMAudioStreamPlayThread)
{
	GMMAudioStreamSource* source = nullptr;
	bool started = false;
	bool terminate = false;
};

class GMMAudioStreamSource : public gm::GMObject, public gm::IAudioSource
{
	DECLARE_PRIVATE(GMMAudioStreamSource)

	friend class GMMAudioStreamPlayThread;

public:
	GMMAudioStreamSource(gm::IAudioFile* file);
	~GMMAudioStreamSource();

public:
	virtual void play(bool loop) override;
	virtual void stop() override;
	virtual void pause() override;
	virtual void rewind() override;
};

class GMMAudioStreamPlayThread : public gm::GMThread
{
	DECLARE_PRIVATE(GMMAudioStreamPlayThread)

public:
	GMMAudioStreamPlayThread(GMMAudioStreamSource* src)
	{
		D(d);
		d->source = src;
	}

	~GMMAudioStreamPlayThread()
	{
		wait();
	}

public:
	virtual void run() override
	{
		D(d_self);
		detach();
		d_self->started = true;

		D_OF(d, d_self->source);

		gm::IAudioStream* stream = d->file->getStream();
		gm::GMuint bufferNum = stream->getBufferNum();

		// 初始化
		gm::GMuint bufferSize = stream->getBufferSize();
		gm::GMbyte* audioData = new gm::GMbyte[bufferSize];

		// 先填充Buffer
		auto& fileInfo = d->file->getFileInfo();
		for (gm::GMuint i = 0; i < bufferNum; ++i)
		{
			stream->readBuffer(audioData);
			alBufferData(d->buffers[i], fileInfo.format, audioData, bufferSize, fileInfo.frequency);
			alSourceQueueBuffers(d->sourceId, 1, &d->buffers[i]);
		}
		alSourcePlay(d->sourceId);

		ALint buffersProcessed = 0;
		ALint totalBuffersProcessed = 0;
		ALuint buffer = 0;
		while (!d_self->terminate)
		{
			gm::GMThread::sleep(1000 / 60);

			alGetSourcei(d->sourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);
			totalBuffersProcessed += buffersProcessed;
			// Check next chunk
			if (buffersProcessed > 0)
				stream->nextChunk(buffersProcessed);
			while (buffersProcessed)
			{
				buffer = 0;
				alSourceUnqueueBuffers(d->sourceId, 1, &buffer);
				if (stream->readBuffer(audioData))
				{
					alBufferData(buffer, fileInfo.format, audioData, bufferSize, fileInfo.frequency);
					alSourceQueueBuffers(d->sourceId, 1, &buffer);
				}

				buffersProcessed--;
			}
		}
		delete[] audioData;
		d_self->started = false;
	}

public:
	bool hasStarted()
	{
		D(d);
		return d->started;
	}

	void terminateThread()
	{
		D(d);
		d->terminate = true;
	}
};

GMMAudioStreamSource::GMMAudioStreamSource(gm::IAudioFile* file)
{
	D(d);
	d->file = file;
	gm::IAudioStream* stream = d->file->getStream();
	d->buffers = new ALuint[stream->getBufferNum()];
	alGenBuffers(stream->getBufferNum(), d->buffers);
	alGenSources(1, &d->sourceId);
	d->thread = new GMMAudioStreamPlayThread(this);
}

GMMAudioStreamSource::~GMMAudioStreamSource()
{
	D(d);
	alDeleteSources(1, &d->sourceId);

	gm::IAudioStream* stream = d->file->getStream();

	if (d->buffers)
	{
		gm::IAudioStream* stream = d->file->getStream();
		alDeleteBuffers(stream->getBufferNum(), d->buffers);
		delete[] d->buffers;
	}

	if (d->thread)
	{
		d->thread->terminateThread();
		delete d->thread;
	}
}

void GMMAudioStreamSource::play(bool loop)
{
	D(d);
	GM_ASSERT(d->file->isStream());
	if (!d->thread->hasStarted())
		d->thread->start();
	else
		alSourcePlay(d->sourceId);
}

void GMMAudioStreamSource::stop()
{
	D(d);
	alSourceStop(d->sourceId);
}

void GMMAudioStreamSource::pause()
{
	D(d);
	alSourcePause(d->sourceId);
}

void GMMAudioStreamSource::rewind()
{
	D(d);
	gm::IAudioStream* stream = d->file->getStream();
	// 停止播放
	alSourceStop(d->sourceId);

	// 停止流解码，回退到流最初状态
	stream->rewind();

	// 停止播放线程
	d->thread->terminateThread();
	d->thread->wait();
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