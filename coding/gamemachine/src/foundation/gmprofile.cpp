#include "stdafx.h"
#include "gmprofile.h"
#include "foundation/gamemachine.h"

//Profile
static IProfileHandler* g_handler = nullptr;

void GMProfile::setHandler(IProfileHandler* handler)
{
	g_handler = handler;
}

void GMProfile::clearHandler()
{
	setHandler(nullptr);
}

GMProfile::GMProfileSessions::GMProfileSession& GMProfile::profileSession()
{
	static GMProfileSessions s;
	return s.sessions[GMThread::getCurrentThreadId()];
}

GMProfile::GMProfile(const GMString& name)
{
	D(d);
	startRecord(name);
}

GMProfile::~GMProfile()
{
	stopRecord();
}

void GMProfile::startRecord(const GMString& name)
{
	D(d);
	GMMutex m;
	static GMLargeInteger frequency = GMClock::highResolutionTimerFrequency();
	if (!GMGetBuiltIn(RUN_PROFILE))
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();
	g_handler->begin(GMThread::getCurrentThreadId(), ps.level);

	d->valid = true;
	d->name = name;
	GMLargeInteger now = GMClock::highResolutionTimer();
	if (ps.firstProfileTimeInCycle == 0)
		ps.firstProfileTimeInCycle = now;
	d->durationSinceLastProfile = (now - ps.firstProfileTimeInCycle) / (GMfloat) frequency;

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
	g_handler->output(d->name, d->stopwatch.timeInSecond(), d->durationSinceLastProfile, id, --level);
	g_handler->end(id, level);
	if (!level)
		ps.firstProfileTimeInCycle = 0;
}
