#include "stdafx.h"
#include "gameobjectprivate.h"
#include "gmgl/gmglgraphic_engine.h"

GameObjectPrivate::GameObjectPrivate()
	: mass(0)
	, isDynamic(true)
	, localInertia(0, 0, 0)
	, world(nullptr)
	, colObj(nullptr)
	, animationStartTick(0)
	, animationDuration(0)
	, animationState(Stopped)
{
	transform.setIdentity();
	memset(&frictions, 0, sizeof(Frictions));
}