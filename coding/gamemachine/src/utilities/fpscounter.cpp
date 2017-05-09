#include "stdafx.h"
#include "fpscounter.h"
#include <time.h>

FPSCounter::FPSCounter()
{
	D(d);
	d.fps = 0;
	d.lastTime = 0.f;
	d.frames = 0.f;
	d.time = 0.f;
	d.immediate_lastTime = 0.f;
}

// 每一帧运行一次update
void FPSCounter::update()
{
	D(d);
	d.time = clock() * 0.001f;								//get current time in seconds
	++d.frames;												//increase frame count

	if (d.time - d.lastTime > 1.0f)							//if it has been 1 second
	{
		d.fps = d.frames / (d.time - d.lastTime);			//update fps number
		d.lastTime = d.time;								//set beginning count
		d.frames = 0L;										//reset frames this second
	}

	d.elapsed_since_last_frame = d.time - d.immediate_lastTime;
	d.immediate_lastTime = d.time;
}

GMfloat FPSCounter::getFps()
{
	D(d);
	return d.fps;
}

GMfloat FPSCounter::getElapsedSinceLastFrame()
{
	D(d);
	return d.elapsed_since_last_frame;
}