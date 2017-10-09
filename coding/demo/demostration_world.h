#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>

typedef Pair<gm::GMString, gm::IGameHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> GameHandlers;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	GameHandlers handlers;
	Vector<gm::GMImage2DGameObject*> entrances;
};

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE(DemostrationWorld)

public:
	DemostrationWorld() = default;
	~DemostrationWorld();

public:
	void init(GameHandlers& handlers);

private:
	void addMenu(GameHandlerItem& item);

private:
	static void mouseDownCallback(GMObject*, GMObject*);
};

#endif