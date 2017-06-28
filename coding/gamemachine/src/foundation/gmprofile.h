#ifndef __GMPROFILE_H__
#define __GMPROFILE_H__
#include "common.h"
#include "gmthreads.h"
#include <stack>
BEGIN_NS

#define GM_PROFILE(name) GMProfile __profile(_L(#name))
#define GM_PROFILE_HANDLER(ptr) GMProfile().setHandler(ptr)
#define GM_PROFILE_CLEAR_HANDLER() GMProfile().clearHandler()

GM_INTERFACE(IProfileHandler)
{
	virtual void begin(GMint id, GMint level) = 0;
	virtual void output(const GMString& name, GMfloat timeInSecond, GMfloat durationSinceStartInSecond, GMint id, GMint level) = 0;
	virtual void end(GMint id, GMint level) = 0;
};

GM_PRIVATE_OBJECT(GMProfile)
{
	GMStopwatch stopwatch;
	GMfloat durationSinceLastProfile = 0; // 距离上一次Profile的时间
	bool valid = false;
	GMString name;
};

class GMProfile : public GMObject
{
	struct GMProfileSessions : public GMObject
	{
		struct GMProfileSession
		{
			GMProfileSession() : level(0) {};
			GMint level;
			GMLargeInteger firstProfileTimeInCycle = 0;
		};

		std::map<GMThreadId, GMProfileSession> sessions;
	};

	DECLARE_PRIVATE(GMProfile)

public:
	static GMProfileSessions::GMProfileSession& profileSession();

public:
	GMProfile() = default;
	GMProfile(const GMString& name);
	~GMProfile();

public:
	void setHandler(IProfileHandler* handler);
	void clearHandler();

private:
	void startRecord(const GMString&  name);
	void stopRecord();
};


END_NS
#endif