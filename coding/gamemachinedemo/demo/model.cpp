#include "stdafx.h"
#include "model.h"
#include <linearmath.h>
#include <gmmodelreader.h>

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
	gm::GMGameObject* baymax = new gm::GMGameObject(asset);

	baymax->setScaling(glm::scale(.01f, .01f, .01f));
	d->demoWorld->addObject("baymax", baymax);
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
