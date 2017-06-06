#ifndef __GMPROFILE_H__
#define __GMPROFILE_H__
#include "common.h"
#include "gmthreads.h"
#include <stack>
BEGIN_NS

#define GM_PROFILE(name) GMProfile __profile(#name)

struct IProfileHandler
{
	virtual ~IProfileHandler() {}
	virtual void write(const char*) = 0;
};

class GMConsoleProfileHandler : public IProfileHandler
{
public:
	virtual void write(const char* str) override
	{
		printf("%s: ", str);
	}
};

GM_PRIVATE_OBJECT(GMProfile)
{
	GMStopwatch stopwatch;
	char name[128];
	bool valid;
};

class GMProfile : public GMObject
{
	struct GMProfileSessions : public GMObject
	{
		struct GMProfileSession
		{
			GMProfileSession() : level(0) {};
			std::stack<std::string> callstack;
			GMint level;
		};

		std::map<GMThreadId, GMProfileSession> sessions;
	};

	DECLARE_PRIVATE(GMProfile)

public:
	static IProfileHandler& handler();
	static GMProfileSessions::GMProfileSession& profileSession();

public:
	GMProfile(const char* name);
	~GMProfile();

private:
	void startRecord(const char* name);
	void stopRecord();
};


END_NS
#endif