#include "stdafx.h"
#include <gmm.h>
#include "gmmdef.h"

void GMMFactory::createAudioPlayer(OUT gm::IAudioPlayer** player)
{
	(*player) = new GMMAudioPlayer();
}