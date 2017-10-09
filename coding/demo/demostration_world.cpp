#include "stdafx.h"
#include "demostration_world.h"

DemostrationWorld::~DemostrationWorld()
{
	D(d);
	for (auto& entrance : d->entrances)
	{
		delete entrance;
	}
}

void DemostrationWorld::init(GameHandlers& handlers)
{
	// ´´½¨Demo²Ëµ¥
	D(d);

	for (auto& handler : d->handlers)
	{
		addMenu(handler);
	}
}

void DemostrationWorld::addMenu(GameHandlerItem& item)
{
	D(d);
	gm::GMImage2DGameObject* entrance = new gm::GMImage2DGameObject();
	entrance->attachEvent(*entrance, gm::GM_CONTROL_EVENT_ENUM(MouseDown), mouseDownCallback);
	// entrance->setText("...");
	d->entrances.push_back(entrance);
}

void DemostrationWorld::mouseDownCallback(GMObject*, GMObject*)
{

}