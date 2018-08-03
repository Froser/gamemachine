#include "stdafx.h"
#include "md5mesh.h"
#include <gmcontrols.h>
#include <gmwidget.h>
#include <gmmodelreader.h>

void Demo_MD5Mesh::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	Base::init();

	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);

	gm::GMModels* boblampcleanModel = nullptr;
	gm::GMModelReader::load(gm::GMModelLoadSettings(L"boblampclean/boblampclean.gmmd5", L"boblampclean"), gm::GMModelReader::Auto, &boblampcleanModel);
	
	auto boblampcleanAsset = gm::GMAssets::createIsolatedAsset(gm::GMAssetType::Models, boblampcleanModel);
	d->boblampclean = new gm::GMGameObject(boblampcleanAsset);
	d->boblampclean->setScaling(Scale(GMVec3(.02f, .02f, .02f)));
	d->boblampclean->setTranslation(Translate(GMVec3(0, -.5f, 0)));
	d->boblampclean->setRotation(Rotate(-PI / 2, GMVec3(1, 0, 0)) * Rotate(PI, GMVec3(0, 1, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("boblampclean", d->boblampclean);
}

void Demo_MD5Mesh::event(gm::GameMachineHandlerEvent evt)
{
	D_BASE(db, Base);
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElpased);
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
