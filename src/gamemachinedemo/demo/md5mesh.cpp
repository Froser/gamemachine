#include "stdafx.h"
#include "md5mesh.h"
#include <gmcontrols.h>
#include <gmwidget.h>
#include <gmmodelreader.h>
#include <gmskeletongameobject.h>

void Demo_MD5Mesh::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	Base::init();

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"切换绘制皮肤",
		10,
		top,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->boblampclean->setDrawSkin(!d->boblampclean->getDrawSkin());
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"切换骨骼",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->boblampclean->setDrawBones(!d->boblampclean->getDrawBones());
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"播放/暂停动画",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->boblampclean->isPlaying())
			d->boblampclean->pause();
		else
			d->boblampclean->play();
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"重置动画",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->boblampclean->reset(true);
	});

	widget->setSize(widget->getSize().width, top + 40);

	gm::GMAsset boblampcleanModel;
	gm::GMModelReader::load(
		gm::GMModelLoadSettings(L"boblampclean/boblampclean.gmmd5", L"boblampclean", db->parentDemonstrationWorld->getContext(), gm::GMModelPathType::Relative),
		gm::GMModelReader::Auto,
		boblampcleanModel
	);
	
	d->boblampclean = new gm::GMSkeletonGameObject(boblampcleanModel);
	d->boblampclean->setScaling(Scale(GMVec3(.02f, .02f, .02f)));
	d->boblampclean->setTranslation(Translate(GMVec3(0, -.5f, 0)));
	d->boblampclean->setRotation(Rotate(-PI / 2, GMVec3(1, 0, 0)) * Rotate(PI, GMVec3(0, 1, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("boblampclean", d->boblampclean);
}

void Demo_MD5Mesh::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* directLight = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Direct, &directLight);
			GM_ASSERT(directLight);
			gm::GMfloat lightPos[] = { 1, 0, -3 };
			directLight->setLightPosition(lightPos);
			gm::GMfloat color[] = { .1f, .1f, .1f };
			directLight->setLightColor(color);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(directLight);
		}

		{
			gm::ILight* ambientLight = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Ambient, &ambientLight);
			GM_ASSERT(ambientLight);
			gm::GMfloat color[] = { .7f, .7f, .7f };
			ambientLight->setLightColor(color);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(ambientLight);
		}
	}
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
