#ifndef __GRAPHIC_ENGINE_H__
#define __GRAPHIC_ENGINE_H__
#include "common.h"
#include "foundation/vector.h"

BEGIN_NS
struct GraphicSettings
{
	GMuint fps;
	GMuint windowSize[2];
	GMuint resolution[2];
	GMuint startPosition[2];
	bool fullscreen;
};

END_NS
#endif