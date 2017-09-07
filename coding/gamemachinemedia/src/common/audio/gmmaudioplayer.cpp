#include "stdafx.h"
#include "gmmaudioplayer.h"
#include "al.h"

GMMAudioPlayer::GMMAudioPlayer()
{
	ALuint r;
	alGenBuffers(1, &r);
}