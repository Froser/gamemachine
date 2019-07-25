#include "stdafx.h"
#include "gmprofile.h"
#include "foundation/gamemachine.h"

BEGIN_NS

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

GM_PRIVATE_OBJECT_UNALIGNED(GMProfile)
{
	GMDebugConfig debugConfig;
	GMStopwatch stopwatch;
	GMfloat durationSinceLastProfile = 0; // 距离上一次Profile的时间
	bool valid = false;
	GMString name;
};

GMProfile::GMProfile(IGraphicEngine* engine)
{
	GM_CREATE_DATA();
	D(d);
	d->debugConfig = engine->getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

GMProfile::GMProfile(IGraphicEngine* engine, const GMString& name)
	: GMProfile(engine)
{
	D(d);
	startRecord(name);
}

GMProfile::GMProfile(IGraphicEngine* engine, const GMwchar* name)
	: GMProfile(engine)
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

END_NS