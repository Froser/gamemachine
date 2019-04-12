#include "stdafx.h"
#include "sponza.h"
#include <gmgameobject.h>
#include <gmmodelreader.h>
#include <gmcontrols.h>
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

void Demo_Sponza::init()
{
	Base::init();
	createMenu();
	createObject();
}

void Demo_Sponza::createMenu()
{
	D(d);
	D_BASE(db, DemoHandler);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Sponza::createObject()
{
	D(d);
	D_BASE(db, Base);
	
	gm::GMModelLoadSettings loadSettings(
		"sponza/sponza.obj",
		getDemoWorldReference()->getContext()
	);

	gm::GMAsset models;
	gm::GMModelReader::load(loadSettings, models);
	gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(models);
	d->sponza = new gm::GMGameObject(asset);
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sponza", d->sponza);
}

void Demo_Sponza::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Activate:
	{
		if (d->activate)
		{
			const static gm::GMfloat mouseSensitivity = 0.25f;
			gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
			gm::IMouseState& mouseState = inputManager->getMouseState();
			auto ms = mouseState.state();
			d->cameraUtility.update(Radian(-ms.deltaX * mouseSensitivity), Radian(-ms.deltaY * mouseSensitivity));

			gm::IKeyboardState& kbState = inputManager->getKeyboardState();
			if (kbState.keyTriggered(gm::GM_ASCIIToKey('R')))
				setMouseTrace(!d->mouseTrace);
		}
		break;
	}
	case gm::GameMachineHandlerEvent::Update:
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Sponza::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookDirection = Normalize(GMVec3(0, 0, 1));
	lookAt.position = GMVec3(0, 10, 0);
	camera.lookAt(lookAt);

	D(d);
	d->cameraUtility.setCamera(&camera);
}

void Demo_Sponza::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
			gm::GMfloat diffuseIntensity[] = { 1.6f, 1.6f, 1.6f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			light->setLightAttribute(gm::GMLight::SpecularIntensity, 1.f);

			gm::GMfloat lightPos[] = { 100.f, 100.f, 100.f };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Sponza::onActivate()
{
	D(d);
	d->activate = true;
	setMouseTrace(true);
	Base::onActivate();
}

void Demo_Sponza::onDeactivate()
{
	D(d);
	d->activate = false;
	setMouseTrace(false);
	Base::onDeactivate();
}

void Demo_Sponza::setMouseTrace(bool enabled)
{
	D(d);
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseTrace = enabled;

	// 鼠标跟踪开启时，detecting mode也开启，每一帧将返回窗口中心，以获取鼠标移动偏量
	mouseState.setDetectingMode(d->mouseTrace);
}