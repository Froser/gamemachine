#include "stdafx.h"
#include "gameobjectprivate.h"

GameObjectPrivate::GameObjectPrivate()
	: world(nullptr)
	, animationStartTick(0)
	, animationDuration(0)
	, animationState(Stopped)
	, id(0)
{
}