#include "stdafx.h"
#include "script.h"
#include "utilities/assert.h"
#include "gmengine/elements/gameworld.h"
#include <map>
#include "common.h"
#include "gmengine/elements/gamelight.h"
#include <algorithm>
#include <fstream>
#include "script_actions.h"
#include "gmengine/elements/gameobject.h"

typedef std::map<std::string, IInvokable* > InvokeList;

#define BEGIN_INVOKABLE(name) \
	struct name##_Invoker : public IInvokable \
	{ \
		virtual void invoke(GameObject* sourceObj, EventItem* evt, GameWorld* world, Arguments& args) override; \
	}; \
	struct name##_Register { \
		name##_Register() { \
			getInvokeList().insert(std::make_pair(#name, new name##_Invoker())); \
		} \
	} name##_Register_instance; \
	void name##_Invoker::invoke(GameObject* sourceObj, EventItem* evt, GameWorld* world, Arguments& args) {
#define EVENT evt
#define SOURCE sourceObj
#define ARGS args
#define ARGS_0 args[0].c_str()
#define ARGS_1 args[1].c_str()
#define ARGS_2 args[2].c_str()
#define ARGS_3 args[3].c_str()
#define ARGS_4 args[4].c_str()
#define WORLD world
#define END_INVOKABLE };

struct _Register
{
	InvokeList invokeList;
	~_Register()
	{
		for (auto iter = invokeList.begin(); iter != invokeList.end(); iter++)
		{
			delete (*iter).second;
		}
	}
};

InvokeList& getInvokeList()
{
	static _Register r;
	return r.invokeList;
}

Script::Script(GameWorld* world)
	: m_world(world)
{
	D(d);
	d.setScript(this);
}

void Script::invoke(GameObject* sourceObj, EventItem* evt, std::string& func, Arguments& args)
{
	InvokeList& invokeList = getInvokeList();
	if (invokeList.find(func) != invokeList.end())
	{
		invokeList[func]->invoke(sourceObj, evt, m_world, args);
	}
	else
	{
		LOG_ASSERT_MSG(false, "Wrong func name.");
	}
}

void Script::loadScript(const char* filename)
{
	D(d);

	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.good())
	{
		char line[LINE_MAX];
		while (file.getline(line, LINE_MAX))
		{
			d.parseLine(line);
		}
		d.parseExpressions();
	}
	else
	{
		LOG_ASSERT_MSG(false, (std::string("Script loading error: ") + filename));
	}
}

// Implements:
BEGIN_INVOKABLE(setLight)
{
	struct _findLight
	{
		_findLight(GMuint id)
			: m_id(id)
		{
		}

		bool operator()(GameLight* l)
		{
			return l->getId() == m_id;
		}

		GMuint m_id;
	};

	GMuint lightID;
	SAFE_SSCANF(ARGS_0, "%d", &lightID);
	GameLight* light = WORLD->getLights() [lightID];
	auto iter = std::find_if(WORLD->getLights().begin(), WORLD->getLights().end(), _findLight(lightID));

	GMfloat r, g, b;
	SAFE_SSCANF(ARGS_1, "%f", &r);
	SAFE_SSCANF(ARGS_2, "%f", &g);
	SAFE_SSCANF(ARGS_3, "%f", &b);

	GMfloat rgb[] = { r, g, b };
	light->setColor(rgb);
}
END_INVOKABLE

BEGIN_INVOKABLE(move)
{
	GMuint id;
	SAFE_SSCANF(ARGS_0, "%d", &id);
	GMfloat duration;
	SAFE_SSCANF(ARGS_1, "%f", &duration);
	GMfloat x, y, z;
	SAFE_SSCANF(ARGS_2, "%f", &x);
	SAFE_SSCANF(ARGS_3, "%f", &y);
	SAFE_SSCANF(ARGS_4, "%f", &z);

	Action_Move* move = new Action_Move(SOURCE, EVENT, duration, btVector3(x, y, z));
	GameObject* obj = WORLD->findGameObjectById(id);
	obj->activateAction(move);
}
END_INVOKABLE