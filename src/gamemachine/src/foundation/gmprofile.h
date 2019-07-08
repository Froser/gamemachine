#ifndef __GMPROFILE_H__
#define __GMPROFILE_H__
#include <gmcommon.h>
#include <gmthread.h>
#include <stack>
BEGIN_NS

#define GM_PROFILE(engine, name) gm::GMProfile __profile(engine, L ## name)
#define GM_PROFILE_HANDLER(ptr) gm::GMProfile::setHandler(ptr)
#define GM_PROFILE_CLEAR_HANDLER() gm::GMProfile::clearHandler()
#define GM_PROFILE_RESET_TIMELINE() gm::GMProfile::resetTimeline()

GM_INTERFACE(IProfileHandler)
{
	virtual void beginProfile(const GMString& name, GMfloat durationSinceStartInSecond, GMThreadId id, GMint32 level) = 0;
	virtual void endProfile(const GMString& name, GMfloat elapsedInSecond, GMThreadId id, GMint32 level) = 0;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMProfile)
{
	GMDebugConfig debugConfig;
	GMStopwatch stopwatch;
	GMfloat durationSinceLastProfile = 0; // 距离上一次Profile的时间
	bool valid = false;
	GMString name;
};

struct GMProfileSessions : public GMObject
{
	struct GMProfileSession
	{
		GMProfileSession() : level(0) {};
		GMint32 level;
		GMint64 firstProfileTimeInCycle = -1;
	};

	Map<GMThreadId, GMProfileSession> sessions;
};

class GMProfile
{
	GM_DECLARE_PRIVATE_NGO(GMProfile)

public:
	static GMProfileSessions::GMProfileSession& profileSession();

public:
	GMProfile(IGraphicEngine* engine);
	GMProfile(IGraphicEngine* engine, const GMString& name);
	GMProfile(IGraphicEngine* engine, const GMwchar* name);
	~GMProfile();

public:
	static void setHandler(IProfileHandler* handler);
	static void clearHandler();
	static void resetTimeline();

private:
	void startRecord(const GMString& name);
	void stopRecord();
};


END_NS
#endif