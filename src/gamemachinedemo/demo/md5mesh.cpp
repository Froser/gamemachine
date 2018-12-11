#include "stdafx.h"
#include "md5mesh.h"
#include <gmcontrols.h>
#include <gmwidget.h>
#include <gmmodelreader.h>
#include <gmskeletalgameobject.h>

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
		L"播放/暂停动画",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
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
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->boblampclean->reset(true);
	});

	widget->setSize(widget->getSize().width, top + 40);

	gm::GMAsset boblampcleanModel;
	gm::GMModelReader::load(
		gm::GMModelLoadSettings(L"boblampclean/boblampclean.md5mesh", db->parentDemonstrationWorld->getContext(), gm::GMModelPathType::Relative),
		gm::GMModelReader::Assimp,
		boblampcleanModel
	);
	
	for (auto& model : boblampcleanModel.getScene()->getModels())
	{
		model.getModel()->getShader().getMaterial().setSpecular(GMVec3(0));
	}

	d->boblampclean = new gm::GMSkeletalGameObject(boblampcleanModel);
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
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat lightPos[] = { 1, 0, -3 };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			gm::GMfloat ambientIntensity[] = { .1f, .1f, .1f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
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
