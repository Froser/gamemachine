#include "stdafx.h"
#include "gmmaudioplayer.h"
#include "al.h"
#include "alc.h"

// 大小保持一致
static_assert(sizeof(gm::GMint) == sizeof(ALenum), "Size error");
static_assert(sizeof(gm::GMint) == sizeof(ALsizei), "Size error");
static ALenum s_alErrCode;
#define GMM_CHECK_AL_ERROR() GM_ASSERT((s_alErrCode = alGetError()) == AL_NO_ERROR);

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

void GMMAudioPlayer::createPlayerHandle(gm::IAudioFile* f)
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
	alSourcePlay(uiSource);

	ALint state = -1;
	do
	{
		Sleep(100);
		// Get Source State
		alGetSourcei(uiSource, AL_SOURCE_STATE, &state);
	} while (state == AL_PLAYING);
}