#ifndef __PROCEDURES_H__
#define __PROCEDURES_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmanimation.h>

using namespace gm;

class Handler;
class Procedures;
struct ProceduresPrivate
{
	ProceduresPrivate(Procedures& p);
	void showLogo(GMDuration dt);

private:
	Procedures& m_procedures;
	GMTextGameObject* m_title;
	GMAnimation m_titleAnimation;
};

class Procedures
{
	friend struct ProceduresPrivate;

public:
	// Procedures
	enum
	{
		ShowLogo = 0,
		Scene,
	};

public:
	Procedures(Handler* handler);

public:
	void run(GMDuration dt);

private:
	Handler* m_handler;
	ProceduresPrivate m_p;
	GMint32 m_procedure;
};

#endif