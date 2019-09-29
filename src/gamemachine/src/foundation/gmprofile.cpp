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
	GM_DECLARE_PUBLIC(GMProfile)
	GMDebugConfig debugConfig;
	GMStopwatch stopwatch;
	GMfloat durationSinceLastProfile = 0; // 距离上一次Profile的时间
	bool valid = false;
	GMString name;
	void startRecord(const GMString& name);
	void stopRecord();
};

GMProfile::GMProfile(IGraphicEngine* engine)
{
	GM_CREATE_DATA();
	GM_SET_PD();
	D(d);
	d->debugConfig = engine->getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

GMProfile::GMProfile(IGraphicEngine* engine, const GMString& name)
	: GMProfile(engine)
{
	D(d);
	d->startRecord(name);
}

GMProfile::GMProfile(IGraphicEngine* engine, const GMwchar* name)
	: GMProfile(engine)
{
	D(d);
	// 这里先判断，阻止赋值GMString
	if (!d->debugConfig.get(GMDebugConfigs::RunProfile_Bool).toBool())
		return;

	d->startRecord(name);
}

GMProfile::~GMProfile()
{
	D(d);
	d->stopRecord();
}

void GMProfilePrivate::startRecord(const GMString& name)
{
	P_D(d);
	static GMint64 frequency = GMClock::highResolutionTimerFrequency();
	if (!g_handler)
		return;

	if (!debugConfig.get(GMDebugConfigs::RunProfile_Bool).toBool())
		return;

	GMProfileSessions::GMProfileSession& ps = d->profileSession();

	valid = true;
	this->name = name;
	GMint64 now = GMClock::highResolutionTimer();
	if (ps.firstProfileTimeInCycle < 0)
		ps.firstProfileTimeInCycle = now;
	GM_ASSERT(now - ps.firstProfileTimeInCycle >= 0);
	durationSinceLastProfile = (now - ps.firstProfileTimeInCycle) / (GMfloat) frequency;
	g_handler->beginProfile(name, durationSinceLastProfile, GMThread::getCurrentThreadId(), ps.level);

	ps.level++;
	stopwatch.start();
}

void GMProfilePrivate::stopRecord()
{
	P_D(pd);
	stopwatch.stop();
	if (!valid)
		return;

	if (!g_handler)
		return;

	GMProfileSessions::GMProfileSession& ps = pd->profileSession();
	GMThreadId id = GMThread::getCurrentThreadId();
	GMint32& level = ps.level;
	g_handler->endProfile(name, stopwatch.timeInSecond(), id, --level);
}

END_NS