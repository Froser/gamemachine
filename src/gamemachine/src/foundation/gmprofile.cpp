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

GMProfile::GMProfile()
{
	D(d);
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

GMProfile::GMProfile(const GMString& name)
	: GMProfile()
{
	D(d);
	startRecord(name);
}

GMProfile::GMProfile(const GMwchar* name)
	: GMProfile()
{
	D(d);
	// 这里先判断，阻止赋值GMString
	if (!d->debugConfig.get(GMDebugConfigs::RunProfile_Bool).toBool())
		return;

	startRecord(name);
}

GMProfile::~GMProfile()
{
	stopRecord();
}

void GMProfile::startRecord(const GMString& name)
{
	D(d);
	static GMint64 frequency = GMClock::highResolutionTimerFrequency();
	if (!g_handler)
		return;

	if (!d->debugConfig.get(GMDebugConfigs::RunProfile_Bool).toBool())
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();

	d->valid = true;
	d->name = name;
	GMint64 now = GMClock::highResolutionTimer();
	if (ps.firstProfileTimeInCycle < 0)
		ps.firstProfileTimeInCycle = now;
	GM_ASSERT(now - ps.firstProfileTimeInCycle >= 0);
	d->durationSinceLastProfile = (now - ps.firstProfileTimeInCycle) / (GMfloat) frequency;
	g_handler->beginProfile(name, d->durationSinceLastProfile, GMThread::getCurrentThreadId(), ps.level);

	ps.level++;
	d->stopwatch.start();
}

void GMProfile::stopRecord()
{
	D(d);
	d->stopwatch.stop();
	if (!d->valid)
		return;

	if (!g_handler)
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();
	GMThreadId id = GMThread::getCurrentThreadId();
	GMint32& level = ps.level;
	g_handler->endProfile(d->name, d->stopwatch.timeInSecond(), id, --level);
}
