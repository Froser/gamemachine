#ifndef __FPSCOUNTER_H__
#define __FPSCOUNTER_H__
#include "common.h"
BEGIN_NS

struct FPSCounterPrivate
{
	GMfloat fps;
	GMfloat lastTime;
	GMlong frames;
	GMfloat time;
};

class FPSCounter
{
	DEFINE_PRIVATE(FPSCounter)

public:
	FPSCounter();
	~FPSCounter() {}

public:
	void update();
	GMfloat getFps();
};


END_NS
#endif