#include "stdafx.h"
#include "animation.h"
#include <gmcontrols.h>
#include <gmwidget.h>
#include <gmmodelreader.h>

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
		if (d->object->isPlaying())
			d->object->pause();
		else
			d->object->play();
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
		d->object->reset(true);
	});

	widget->setSize(widget->getSize().width, top + 40);

	gm::GMAsset objectModel;
	gm::GMModelReader::load(
		gm::GMModelLoadSettings(L"boblampclean/boblampclean.md5mesh", db->parentDemonstrationWorld->getContext(), gm::GMModelPathType::Relative),
		gm::GMModelReader::Assimp,
		objectModel
	);
	
	for (auto& model : objectModel.getScene()->getModels())
	{
		model.getModel()->getShader().getMaterial().setSpecular(GMVec3(0));
	}

	d->object = new gm::GMGameObject(objectModel);
	d->object->setScaling(Scale(GMVec3(.02f, .02f, .02f)));
	d->object->setTranslation(Translate(GMVec3(0, -.5f, 0)));
	d->object->setRotation(Rotate(-PI / 2, GMVec3(1, 0, 0)) * Rotate(PI, GMVec3(0, 1, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("object", d->object);
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

//////////////////////////////////////////////////////////////////////////
void Demo_MultiAnimation::init()
{
	D(d);
	D_BASE(db, Base);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	Base::init();

	gm::GMAsset objectModel;
	gm::GMModelReader::load(
		gm::GMModelLoadSettings(L"multiani/multiani.fbx", db->parentDemonstrationWorld->getContext(), gm::GMModelPathType::Relative),
		gm::GMModelReader::Assimp,
		objectModel
	);

	for (auto& model : objectModel.getScene()->getModels())
	{
		model.getModel()->getShader().getMaterial().setSpecular(GMVec3(0));
	}

	d->object = new gm::GMGameObject(objectModel);
	auto cnt = d->object->getAnimationCount();

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"播放动画anim 1",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		auto index = d->object->getAnimationIndexByName("anim 1");
		if (index != gm::GMGameObject::InvalidIndex)
			d->object->setAnimation(index);
		d->object->reset(true);
		d->object->play();
	});
	button->emitSignal(GM_SIGNAL(gm::GMControlButton, click));

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"播放动画anim 2",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		auto index = d->object->getAnimationIndexByName("anim 2");
		if (index != gm::GMGameObject::InvalidIndex)
			d->object->setAnimation(index);
		d->object->reset(true);
		d->object->play();
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"播放动画spin",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		auto index = d->object->getAnimationIndexByName("spin");
		if (index != gm::GMGameObject::InvalidIndex)
			d->object->setAnimation(index);
		d->object->reset(true);
		d->object->play();
	});

	widget->setSize(widget->getSize().width, top + 40);

	d->object->setScaling(Scale(GMVec3(.02f, .02f, .02f)));
	asDemoGameWorld(getDemoWorldReference())->addObject("object", d->object);
}

void Demo_MultiAnimation::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat lightPos[] = { -2, 2, 2 };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			gm::GMfloat ambientIntensity[] = { .3f, .3f, .3f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_MultiAnimation::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.lookAt(gm::GMCameraLookAt::makeLookAt(GMVec3(-1, .5f, 0), GMVec3(2.35f, 0, 1.85f)));
}
