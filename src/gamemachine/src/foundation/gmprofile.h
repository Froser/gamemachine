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

GM_PRIVATE_CLASS(GMProfile);
class GMProfile : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMProfile)

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