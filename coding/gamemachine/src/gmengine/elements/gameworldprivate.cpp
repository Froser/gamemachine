#include "stdafx.h"
#include "gameworldprivate.h"
#include "gameobject.h"
#include "utilities/assert.h"
#include "gmgl/gmglgraphic_engine.h"
#include <algorithm>

GameWorldPrivate::GameWorldPrivate()
	: character(nullptr)
	, sky(nullptr)
	, ellapsed(0)
	, upAxis(Y_AXIS)
{

}