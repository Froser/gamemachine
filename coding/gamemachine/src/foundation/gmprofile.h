#ifndef __GMPROFILE_H__
#define __GMPROFILE_H__
#include "common.h"
#include "gmthreads.h"
#include <stack>
BEGIN_NS

#define GM_PROFILE(name) GMProfile __profile(#name)
#define GM_PROFILE_HANDLER(ptr) GMProfile().setHandler(ptr)

GM_INTERFACE(IProfileHandler)
{
	virtual void begin(GMint id, GMint level) = 0;
	virtual void output(const GMString& name, GMfloat timeInSecond, GMint id, GMint level) = 0;
	virtual void end(GMint id, GMint level) = 0;
};

GM_PRIVATE_OBJECT(GMProfile)
{
	GMStopwatch stopwatch;
	char name[128];
	bool valid = false;
};

class GMProfile : public GMObject
{
	struct GMProfileSessions : public GMObject
	{
		struct GMProfileSession
		{
			GMProfileSession() : level(0) {};
			std::stack<GMString> callstack;
			GMint level;
		};

		std::map<GMThreadId, GMProfileSession> sessions;
	};

	DECLARE_PRIVATE(GMProfile)

public:
	static GMProfileSessions::GMProfileSession& profileSession();

public:
	GMProfile() = default;
	GMProfile(const char* name);
	~GMProfile();

public:
	void setHandler(IProfileHandler* handler);

private:
	void startRecord(const char* name);
	void stopRecord();
};


END_NS
#endif