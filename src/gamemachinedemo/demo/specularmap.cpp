#include "stdafx.h"
#include "specularmap.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, -.2f, 1),
		GMVec3(0, .2f, 0)
	);
}

void Demo_SpecularMap::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_SpecularMap::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	{
		gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "cube_diffuse.png");
		getDemoWorldReference()->getAssets().addAsset(texture);

		gm::GMSceneAsset cube;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), cube);

		gm::GMShader& shader = cube.getScene()->getModels()[0].getModel()->getShader();
		shader.getMaterial().setSpecular(GMVec3(1));
		shader.getMaterial().setDiffuse(GMVec3(1));
		shader.getMaterial().setShininess(99);
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Diffuse);

		texture = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "cube_specular.png");
		getDemoWorldReference()->getAssets().addAsset(texture);
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Specular);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(cube);
		d->gameObject = new gm::GMGameObject(asset);
		d->gameObject->setTranslation(Translate(GMVec3(-.2f, 0, .5f)));
		d->gameObject->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}

	{
		gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "cube_diffuse.png");
		getDemoWorldReference()->getAssets().addAsset(texture);

		gm::GMSceneAsset cube;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), cube);

		gm::GMShader& shader = cube.getScene()->getModels()[0].getModel()->getShader();
		shader.getMaterial().setSpecular(GMVec3(1));
		shader.getMaterial().setDiffuse(GMVec3(1));
		shader.getMaterial().setShininess(99);
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Diffuse);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(cube);
		d->gameObject2 = new gm::GMGameObject(asset);
		d->gameObject2->setTranslation(Translate(GMVec3(.2f, 0, .5f)));
		d->gameObject2->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject2->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}

	asDemoGameWorld(getDemoWorldReference())->addObject("cube with specular map", d->gameObject);
	asDemoGameWorld(getDemoWorldReference())->addObject("cube without specular map", d->gameObject2);

	createDefaultWidget();
}

void Demo_SpecularMap::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
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
			{
				d->gameObject->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
				d->gameObject2->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
			}
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

void Demo_SpecularMap::handleDragging()
{
	D(d);
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.mouseState();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;

		GMQuat q = Rotate(d->gameObject->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->gameObject->setRotation(q);
		d->gameObject2->setRotation(q);

		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
	else if (d->draggingR)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;
		gm::GMfloat rotateY = d->mouseDownY - state.posY;
		GMVec3 lookAt3 = Normalize(s_lookAt.lookAt);
		GMVec4 lookAt = GMVec4(lookAt3, 1.f);
		GMQuat q = Rotate(d->lookAtRotation,
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->lookAtRotation = q;
		q = Rotate(d->lookAtRotation,
			PI * rotateY / windowStates.renderRect.width,
			GMVec3(1, 0, 0));
		d->lookAtRotation = q;
		gm::GMCameraLookAt cameraLookAt = {
			GMVec4(s_lookAt.lookAt, 1.f) * QuatToMatrix(q),
			s_lookAt.position
		};
		getDemonstrationWorld()->getContext()->getEngine()->getCamera().lookAt(cameraLookAt);
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
}

void Demo_SpecularMap::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		gm::ILight* light = nullptr;
		GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
		GM_ASSERT(light);
		gm::GMfloat lightPos[] = { 0, 0, 0 };
		light->setLightAttribute3(gm::GMLight::Position, lightPos);
		getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
	}
}

void Demo_SpecularMap::event(gm::GameMachineHandlerEvent evt)
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
