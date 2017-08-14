#include "stdafx.h"
#include "gmprofile.h"
#include "foundation/gamemachine.h"

//Profile
static IProfileHandler* g_handler = nullptr;
static GMProfileSessions g_sessions;

void GMProfile::setHandler(IProfileHandler* handler)
{
	g_handler = handler;
}

void GMProfile::clearHandler()
{
	setHandler(nullptr);
}

void GMProfile::resetTimeline()
{
	for (auto& session : g_sessions.sessions)
	{
		session.second.firstProfileTimeInCycle = -1;
	}
}

GMProfileSessions::GMProfileSession& GMProfile::profileSession()
{
	return g_sessions.sessions[GMThread::getCurrentThreadId()];
}

GMProfile::GMProfile(const GMWchar* name)
{
	D(d);
	startRecord(name);
}

GMProfile::~GMProfile()
{
	stopRecord();
}

void GMProfile::startRecord(const GMWchar* name)
{
	D(d);
	GMMutex m;
	static GMLargeInteger frequency = GMClock::highResolutionTimerFrequency();
	if (!GMGetBuiltIn(RUN_PROFILE))
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();

	d->valid = true;
	
	wcscpy_s(d->name, name);
	GMLargeInteger now = GMClock::highResolutionTimer();
	if (ps.firstProfileTimeInCycle < 0)
		ps.firstProfileTimeInCycle = now;
	ASSERT(now - ps.firstProfileTimeInCycle >= 0);
	d->durationSinceLastProfile = (now - ps.firstProfileTimeInCycle) / (GMfloat) frequency;
	g_handler->beginProfile(name, d->durationSinceLastProfile, GMThread::getCurrentThreadId(), ps.level);

	ps.level++;
	d->stopwatch.start();
}

void GMProfile::stopRecord()
{
	GMMutex m;
	D(d);
	d->stopwatch.stop();
	if (!d->valid)
		return;

	if (!g_handler)
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();
	GMint id = GMThread::getCurrentThreadId();
	GMint& level = ps.level;
	g_handler->endProfile(d->name, d->stopwatch.timeInSecond(), id, --level);
}
