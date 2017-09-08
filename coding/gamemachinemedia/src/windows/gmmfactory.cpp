#include "stdafx.h"
#include <gmm.h>
#include "gmmdef.h"

void GMMFactory::createAudioReader(OUT gm::IAudioReader** reader)
{
	(*reader) = new GMMAudioReader();
}

void GMMFactory::createAudioPlayer(OUT gm::IAudioPlayer** player)
{
	(*player) = new GMMAudioPlayer();
}