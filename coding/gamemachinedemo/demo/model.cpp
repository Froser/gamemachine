#include "stdafx.h"
#include "model.h"
#include <linearmath.h>
#include <gmmodelreader.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

Demo_Model::~Demo_Model()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Model::setLookAt()
{
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);
	GM.getCamera().synchronizeLookAt();
}

void Demo_Model::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();
	
	gm::GMGamePackage& pk = *GM.getGamePackageManager();

	gm::GMModelLoadSettings loadSettings = {
		pk,
		pk.pathOf(gm::GMPackageIndex::Models, "baymax/baymax.obj"),
		"baymax"
	};

	gm::GMModel* model = new gm::GMModel();
	gm::GMModelReader::load(loadSettings, &model);

	// 交给GameWorld管理资源
	gm::GMAsset asset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, model);
	d->gameObject = new gm::GMGameObject(asset);

	d->gameObject->setScaling(glm::scale(.01f, .01f, .01f));
	d->gameObject->setTranslation(glm::translate(glm::vec3(0, -.2f, 0)));
	d->demoWorld->addObject("baymax", d->gameObject);
}

void Demo_Model::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.wheel.wheeled)
	{
		gm::GMfloat delta = .001f * state.wheel.delta / WHEEL_DELTA;
		gm::GMfloat scaling[4];
		glm::getScalingFromMatrix(d->gameObject->getScaling(), scaling);
		scaling[0] += delta;
		scaling[1] += delta;
		scaling[2] += delta;

		if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
			d->gameObject->setScaling(glm::scale(scaling[0], scaling[1], scaling[2]));
	}
}

void Demo_Model::event(gm::GameMachineEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		handleMouseEvent();
		d->demoWorld->simulateGameWorld();
		break;
	case gm::GameMachineEvent::Render:
		d->demoWorld->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
		d->demoWorld->notifyControls();
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}
