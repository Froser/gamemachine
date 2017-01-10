#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__
#include "common.h"
BEGIN_NS
class Stopwatch
{
public:
	Stopwatch();
	void start();
	void stop();
	GMint getMillisecond();
	GMint getElapsedMillisecond();
	bool isStarted();

private:
	GMint m_start;
	GMint m_end;
	bool m_bStart;
};
END_NS
#endif