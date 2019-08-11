#include "stdafx.h"
#include "effectreader.h"
#include <linearmath.h>
#include <gmeffectgameobject.h>
#include <gmcontrols.h>

Demo_EffectReader::Demo_EffectReader(DemonstrationWorld* parentDemonstrationWorld)
	: Base(parentDemonstrationWorld)
{
	GM_CREATE_DATA();
}

void Demo_EffectReader::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(gm::GMCameraLookAt::makeLookAt(GMVec3(0, 1, -1), Zero<GMVec3>()));
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
	d->obj->setUniform(L"lightColor", GMVec3(.7f, 0, 0));

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"红光",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->obj->setUniform(L"lightColor", GMVec3(.7f, 0, 0));
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"绿光",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->obj->setUniform(L"lightColor", GMVec3(0, .7f, 0));
	});

	widget->setSize(widget->getSize().width, top + 40);
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
	{
		d->degree += .1f;
		d->obj->setRotation(Rotate(Radians(d->degree), GMVec3(0, 1, 0)));
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElapsed);
		break;
	}
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
