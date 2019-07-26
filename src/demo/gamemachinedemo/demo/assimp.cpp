#include "stdafx.h"
#include "assimp.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>
#include <gmwidget.h>
#include <gmcontrols.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, -.3f, .5f),
		GMVec3(0, .4f, -.5f)
	);
}

void Demo_Assimp::onDeactivate()
{
	D(d);
	D_BASE(db, Base);
	db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);
	DemoHandler::onDeactivate();
}

void Demo_Assimp::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}


Demo_Assimp::Demo_Assimp(DemonstrationWorld* parentDemonstrationWorld)
	: Base(parentDemonstrationWorld)
{
	GM_CREATE_DATA();
}

void Demo_Assimp::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "bnp.png");
	getDemoWorldReference()->getAssets().addAsset(texture);

	gm::GMGamePackage& pk = *GM.getGamePackageManager();
	gm::GMModelLoadSettings loadSettings(
		"boblampclean/boblampclean.md5mesh",
		getDemoWorldReference()->getContext()
	);

	gm::GMAsset models;
	gm::GMModelReader::load(loadSettings, gm::GMModelReader::Assimp, models);

	gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(models);
	d->gameObject = new gm::GMGameObject(asset);
	d->gameObject->setTranslation(Translate(GMVec3(0.f, .25f, 0)));
	d->gameObject->setScaling(Scale(GMVec3(.015f, .015f, .015f)));
	d->gameObject->setRotation(Rotate(PI, GMVec3(0, 1, 0)));

	asDemoGameWorld(getDemoWorldReference())->addObject("obj0", d->gameObject);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Assimp::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	if (state.wheeled)
	{
		gm::GMfloat delta = .001f * state.wheeledDelta / WHEEL_DELTA;
		GMFloat4 scaling;
		{
			GetScalingFromMatrix(d->gameObject->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->gameObject->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
		}
	}

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
	if (state.downButton & gm::GMMouseButton_Right)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingR = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Right)
	{
		d->draggingR = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}

void Demo_Assimp::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;

		GMQuat q = Rotate(d->gameObject->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->gameObject->setRotation(q);

		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
	else if (d->draggingR)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;
		gm::GMfloat rotateY = d->mouseDownY - state.posY;
		GMVec3 lookDir3 = Normalize(s_lookAt.lookDirection);
		GMQuat q = Rotate(d->lookAtRotation,
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->lookAtRotation = q;
		q = Rotate(d->lookAtRotation,
			PI * rotateY / windowStates.renderRect.width,
			GMVec3(1, 0, 0));
		d->lookAtRotation = q;
		gm::GMCameraLookAt cameraLookAt = {
			GMVec4(s_lookAt.lookDirection, 1.f) * QuatToMatrix(q),
			s_lookAt.position
		};
		getDemonstrationWorld()->getContext()->getEngine()->getCamera().lookAt(cameraLookAt);
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
}

void Demo_Assimp::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat lightPos[] = { .7f, .7f, -.7f };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Assimp::event(gm::GameMachineHandlerEvent evt)
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
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElapsed);
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
	{
		handleMouseEvent();
		handleDragging();
		break;
	}
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}
