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
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_SpecularMap::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference() = new gm::GMDemoGameWorld();

	{
		gm::ITexture* texture = nullptr;
		gm::GMToolUtil::createTexture("cube_diffuse.png", &texture);
		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, texture);

		gm::GMModel* cube = nullptr;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), &cube);

		gm::GMShader& shader = cube->getShader();
		shader.getMaterial().ks = GMVec3(1);
		shader.getMaterial().kd = GMVec3(1);
		shader.getMaterial().shininess = 99;
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Diffuse);

		gm::GMToolUtil::createTexture("cube_specular.png", &texture);
		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, texture);
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Specular);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Model, cube);
		d->gameObject = new gm::GMGameObject(asset);
		d->gameObject->setTranslation(Translate(GMVec3(-.2f, 0, .5f)));
		d->gameObject->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}

	{
		gm::ITexture* texture = nullptr;
		gm::GMToolUtil::createTexture("cube_diffuse.png", &texture);
		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, texture);

		gm::GMModel* cube = nullptr;
		gm::GMPrimitiveCreator::createCube(gm::GMPrimitiveCreator::one3(), &cube);

		gm::GMShader& shader = cube->getShader();
		shader.getMaterial().ks = GMVec3(1);
		shader.getMaterial().kd = GMVec3(1);
		shader.getMaterial().shininess = 99;
		gm::GMToolUtil::addTextureToShader(shader, texture, gm::GMTextureType::Diffuse);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Model, cube);
		d->gameObject2 = new gm::GMGameObject(asset);
		d->gameObject2->setTranslation(Translate(GMVec3(.2f, 0, .5f)));
		d->gameObject2->setScaling(Scale(GMVec3(.1f, .1f, .1f)));
		d->gameObject2->setRotation(Rotate(PI, GMVec3(0, 1, 0)));
	}

	asDemoGameWorld(getDemoWorldReference())->addObject("cube with specular map", d->gameObject);
	asDemoGameWorld(getDemoWorldReference())->addObject("cube without specular map", d->gameObject2);
}

void Demo_SpecularMap::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.wheel.wheeled)
	{
		gm::GMfloat delta = .001f * state.wheel.delta / WHEEL_DELTA;
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

	if (state.downButton & GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & GMMouseButton_Left)
	{
		d->draggingL = false;
		GM.getMainWindow()->setLockWindow(false);
	}
	if (state.downButton & GMMouseButton_Right)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingR = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & GMMouseButton_Right)
	{
		d->draggingR = false;
		GM.getMainWindow()->setLockWindow(false);
	}
}

gm::GMCubeMapGameObject* Demo_SpecularMap::createCubeMap()
{
	gm::GMGamePackage* pk = GM.getGamePackageManager();
	gm::GMImage* slices[6] = { nullptr };
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posx.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[0]);
		GM_ASSERT(slices[0]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negx.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[1]);
		GM_ASSERT(slices[1]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posy.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[2]);
		GM_ASSERT(slices[2]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negy.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[3]);
		GM_ASSERT(slices[3]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posz.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[4]);
		GM_ASSERT(slices[4]);
	}
	{
		gm::GMBuffer buf;
		pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negz.jpg", &buf);
		gm::GMImageReader::load(buf.buffer, buf.size, &slices[5]);
		GM_ASSERT(slices[5]);
	}

	gm::GMCubeMapBuffer cubeMap(*slices[0], *slices[1], *slices[2], *slices[3], *slices[4], *slices[5]);
	gm::ITexture* cubeMapTex = nullptr;
	GM.getFactory()->createTexture(&cubeMap, &cubeMapTex);

	for (auto slice : slices)
	{
		gm::GM_delete(slice);
	}

	return new gm::GMCubeMapGameObject(cubeMapTex);
}

void Demo_SpecularMap::handleDragging()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;

		GMQuat q = Rotate(d->gameObject->getRotation(),
			PI * rotateX / GM.getGameMachineRunningStates().renderRect.width,
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
			PI * rotateX / GM.getGameMachineRunningStates().renderRect.width,
			GMVec3(0, 1, 0));
		d->lookAtRotation = q;
		q = Rotate(d->lookAtRotation,
			PI * rotateY / GM.getGameMachineRunningStates().renderRect.width,
			GMVec3(1, 0, 0));
		d->lookAtRotation = q;
		gm::GMCameraLookAt cameraLookAt = {
			GMVec4(s_lookAt.lookAt, 1.f) * QuatToMatrix(q),
			s_lookAt.position
		};
		GM.getCamera().lookAt(cameraLookAt);
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
		GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
		GM_ASSERT(light);
		gm::GMfloat lightPos[] = { 0, 0, 0 };
		light->setLightPosition(lightPos);
		gm::GMfloat color[] = { 1, 1, 1 };
		light->setLightColor(color);
		GM.getGraphicEngine()->addLight(light);
	}
}

void Demo_SpecularMap::event(gm::GameMachineEvent evt)
{
	D_BASE(db, Base);
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		getDemoWorldReference()->simulateGameWorld();
		break;
	case gm::GameMachineEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
	{
		handleMouseEvent();
		handleDragging();
		getDemoWorldReference()->notifyControls();

		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('N'))
			switchNormal();

		break;
	}
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}
