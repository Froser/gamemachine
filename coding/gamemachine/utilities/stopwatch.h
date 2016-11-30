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
	Fint getMillisecond();
	bool isStarted();

private:
	Fint m_start;
	Fint m_end;
	bool m_bStart;
};
END_NS
#endif