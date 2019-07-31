#include "stdafx.h"
#include "effectreader.h"
#include <linearmath.h>
#include <gmeffectgameobject.h>

Demo_EffectReader::Demo_EffectReader(DemonstrationWorld* parentDemonstrationWorld)
	: Base(parentDemonstrationWorld)
{
	GM_CREATE_DATA();
}

void Demo_EffectReader::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	GM_ASSERT(!getDemoWorldReference());
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	gm::GMPrimitiveCreator::createCube(GMVec3(.2f, .2f, .2f), d->cubeScene);
	d->obj = db->parentDemonstrationWorld->getEffectObjectFactory()->createGameObject(d->cubeScene);
	asDemoGameWorld(getDemoWorldReference())->addObject("box", d->obj);

	createDefaultWidget();
}

void Demo_EffectReader::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElapsed);
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}