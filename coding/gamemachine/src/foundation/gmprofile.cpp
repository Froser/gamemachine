#include "stdafx.h"
#include "gmprofile.h"
#include "foundation/gamemachine.h"

//Profile
IProfileHandler& GMProfile::handler()
{
	static GMConsoleProfileHandler h;
	return h;
}

GMProfile::GMProfileSessions::GMProfileSession& GMProfile::profileSession()
{
	static GMProfileSessions s;
	return s.sessions[GMThread::getCurrentThreadId()];
}

GMProfile::GMProfile(const char* name)
{
	D(d);
	d->valid = false;
	startRecord(name);
}

GMProfile::~GMProfile()
{
	stopRecord();
}

void GMProfile::startRecord(const char* name)
{
	D(d);
	if (!GMGetBuiltIn(RUN_PROFILE))
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();
	ps.level++;

	d->valid = true;
	strcpy_s(d->name, name);
	ps.callstack.push(d->name);
	d->stopwatch.start();
}

void GMProfile::stopRecord()
{
	D(d);
	d->stopwatch.stop();
	if (!d->valid)
		return;

	GMProfileSessions::GMProfileSession& ps = profileSession();
	GMint level = ps.level;
	while (--level)
	{
		handler().write(" ");
	}

	char report[512];
	sprintf_s(report, "'%s' : %f s\n", d->name, d->stopwatch.timeInSecond());
	ps.level--;
	ps.callstack.pop();
	handler().write(report);
}
