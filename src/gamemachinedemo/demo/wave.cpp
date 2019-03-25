#include "stdafx.h"
#include "wave.h"
#include <extensions/objects/gmwavegameobject.h>
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

void Demo_Wave::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	createDefaultWidget();

	gm::GMWaveGameObjectDescription desc = {
		-256.f,
		-256.f,
		512.f,
		512.f,
		80.f,
		25,
		25,
		100,
		100
	};

	gm::GMWaveGameObject* wave = gm::GMWaveGameObject::create(desc);
	GMVec3 direction1 = Normalize(GMVec3(1, 0, 1));
	GMVec3 direction2 = Normalize(GMVec3(-1, 0, -1));
	GMVec3 direction3 = Normalize(GMVec3(-1, 0, 0));
	Vector<gm::GMWaveDescription> wd = {
		{ 0.f, 5.f, direction1, 5.f, 7.f },
		{ 0.01f, 2.f, direction2, 5.f, 5.f },
		//{ 0.01f, 3.f, direction3, 7.f, 3.f },
	};
	wave->setWaveDescriptions(wd);
	wave->play();
	d->wave = wave;
	gm::GMModel* waveModel = d->wave->getModel();
	gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(db->parentDemonstrationWorld->getContext(), L"water.tga");
	gm::GMToolUtil::addTextureToShader(waveModel->getShader(), texture, gm::GMTextureType::Ambient);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapS(gm::GMS_Wrap::Repeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapT(gm::GMS_Wrap::Repeat);
	waveModel->getShader().getMaterial().setAmbient(GMVec3(.3f, .3f, .3f));

	asDemoGameWorld(getDemoWorldReference())->addObject(L"wave", d->wave);
}

void Demo_Wave::event(gm::GameMachineHandlerEvent evt)
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
		if (d->wave)
			d->wave->update(GM.getRunningStates().lastFrameElpased);
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		handleMouseEvent();
		handleDragging();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Wave::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookDirection = Normalize(GMVec3(0, -.5f, 1));
	lookAt.position = GMVec3(0, 100, 0);
	camera.lookAt(lookAt);
}

void Demo_Wave::setDefaultLights()
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
			gm::GMfloat ambientIntensity[] = { .5f, .5f, .5f };
			gm::GMfloat diffuseIntensity[] = { .3f, .3f, .3f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			light->setLightAttribute(gm::GMLight::SpecularIntensity, .3f);

			gm::GMfloat lightPos[] = { 100.f, 100.f, 100.f };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Wave::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
	if (state.wheeled)
	{
		gm::GMfloat delta = .05f * state.wheeledDelta / WHEEL_DELTA;
		GMFloat4 scaling;
		{
			GetScalingFromMatrix(d->wave->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->wave->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
		}
	}

	if (d->draggingL)
	{
		gm::GMfloat rotateX = state.posX - d->mouseDownX;
		GMQuat q = Rotate(d->wave->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->wave->setRotation(q);
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
}

void Demo_Wave::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->draggingL = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}