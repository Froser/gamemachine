#include "stdafx.h"
#include "gmprofile.h"
#include "foundation/gamemachine.h"

//Profile
static IProfileHandler* g_handler = nullptr;

void GMProfile::setHandler(IProfileHandler* handler)
{
	g_handler = handler;
}

GMProfile::GMProfileSessions::GMProfileSession& GMProfile::profileSession()
{
	static GMProfileSessions s;
	return s.sessions[GMThread::getCurrentThreadId()];
}

GMProfile::GMProfile(const char* name)
{
	D(d);
	startRecord(name);
}

GMProfile::~GMProfile()
{
	stopRecord();
}

void GMProfile::startRecord(const char* name)
{
	D(d);
	GMMutex m;
	if (!GMGetBuiltIn(RUN_PROFILE))
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();

	g_handler->begin(GMThread::getCurrentThreadId(), ps.level);
	ps.level++;

	d->valid = true;
	strcpy_s(d->name, name);
	ps.callstack.push(d->name);
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
	g_handler->output(d->name, d->stopwatch.timeInSecond(), id, --level);
	g_handler->end(id, level);
	ps.callstack.pop();
}
