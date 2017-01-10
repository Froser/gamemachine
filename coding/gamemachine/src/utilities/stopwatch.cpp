#include "stdafx.h"
#include "stopwatch.h"
#include <time.h>

Stopwatch::Stopwatch()
	: m_bStart(false)
{

}

void Stopwatch::start()
{
	m_bStart = true;
	m_start = clock();
}

void Stopwatch::stop()
{
	m_bStart = false;
	m_end = clock();
}

GMint Stopwatch::getMillisecond()
{
	return m_end - m_start;
}

GMint Stopwatch::getElapsedMillisecond()
{
	return clock() - m_start;
}

bool Stopwatch::isStarted()
{
	return m_bStart;
}