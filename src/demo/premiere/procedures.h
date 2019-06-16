#ifndef __PROCEDURES_H__
#define __PROCEDURES_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmanimation.h>
#include "timeline.h"

using namespace gm;

class Handler;
class Procedures;
struct ProceduresPrivate
{
	ProceduresPrivate(Procedures& p);
	void finalize();
	void showLogo(GMDuration dt);
	void loadingScene(GMDuration dt);
	void play(GMDuration dt);

	void invokeInMainThread(std::function<void()>);
	void callMainThreadFunctions();
	void async(GMFuture<void>&& asyncFuture);

private:
	Procedures& m_procedures;
	GMTextGameObject* m_title;
	Stack<std::function<void()>> m_funcQueue;
	List<GMFuture<void>> m_managedFutures;
	GMAnimation m_titleAnimation;
	Timeline m_timeline;
	bool m_assetsLoaded;
	bool m_timelineStarted;
};

class Procedures
{
	friend struct ProceduresPrivate;

public:
	// Procedures
	enum
	{
		ShowLogo = 0,
		LoadingScene,
		Play,
	};

public:
	Procedures(Handler* handler);

public:
	void run(GMDuration dt);
	void finalize();

private:
	Handler* m_handler;
	ProceduresPrivate m_p;
	GMint32 m_procedure;
};

#endif