#ifndef __GMPROFILE_H__
#define __GMPROFILE_H__
#include "common.h"
#include "gmthreads.h"
#include <stack>
BEGIN_NS

#define GM_PROFILE(name) GMProfile __profile(_L(#name))
#define GM_PROFILE_HANDLER(ptr) GMProfile::setHandler(ptr)
#define GM_PROFILE_CLEAR_HANDLER() GMProfile::clearHandler()
#define GM_PROFILE_RESET_TIMELINE() GMProfile::resetTimeline()

GM_INTERFACE(IProfileHandler)
{
	virtual void beginProfile(const GMString& name, GMfloat durationSinceStartInSecond, GMint id, GMint level) = 0;
	virtual void endProfile(const GMString& name, GMfloat elapsedInSecond, GMint id, GMint level) = 0;
};

GM_PRIVATE_OBJECT(GMProfile)
{
	GMStopwatch stopwatch;
	GMfloat durationSinceLastProfile = 0; // 距离上一次Profile的时间
	bool valid = false;
	GMWchar name[128];
};

struct GMProfileSessions : public GMObject
{
	struct GMProfileSession
	{
		GMProfileSession() : level(0) {};
		GMint level;
		GMLargeInteger firstProfileTimeInCycle = -1;
	};

	Map<GMThreadId, GMProfileSession> sessions;
};

class GMProfile : public GMObject
{
	DECLARE_PRIVATE(GMProfile)

public:
	static GMProfileSessions::GMProfileSession& profileSession();

public:
	GMProfile() = default;
	GMProfile(const GMWchar* name);
	~GMProfile();

public:
	static void setHandler(IProfileHandler* handler);
	static void clearHandler();
	static void resetTimeline();

private:
	void startRecord(const GMWchar* name);
	void stopRecord();
};


END_NS
#endif