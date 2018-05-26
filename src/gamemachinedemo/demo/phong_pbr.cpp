#include "stdafx.h"
#include "phong_pbr.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, 0, 1),
		GMVec3(0, 0, -1)
	);
}

void Demo_Phong_PBR::setLookAt()
{
	D(d);
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_Phong_PBR::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference() = new gm::GMDemoGameWorld();

	{
		gm::GMModel* sphere = nullptr;
		gm::GMPrimitiveCreator::createSphere(1.0f, 64, 64, &sphere);
		gm::GMShader& shader = sphere->getShader();
		shader.setIlluminationModel(gm::GMIlluminationModel::CookTorranceBRDF);
		gm::ITexture* albedo = nullptr;
		gm::ITexture* metallicRoughnessAO = nullptr;
		gm::ITexture* normal = nullptr;
		gm::GMToolUtil::createPBRTextures(
			"pbr/albedo.png",
			"pbr/metallic.png",
			"pbr/roughness.png",
			"",
			"pbr/normal.png",
			&albedo,
			&metallicRoughnessAO,
			&normal
		);

		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, albedo);
		gm::GMToolUtil::addTextureToShader(shader, albedo, gm::GMTextureType::Albedo);

		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, normal);
		gm::GMToolUtil::addTextureToShader(shader, normal, gm::GMTextureType::NormalMap);

		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, metallicRoughnessAO);
		gm::GMToolUtil::addTextureToShader(shader, metallicRoughnessAO, gm::GMTextureType::MetallicRoughnessAO);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Model, sphere);
		d->gameObject = new gm::GMGameObject(asset);
		d->gameObject->setTranslation(Translate(GMVec3(-.66f, 0, .5f)));
		d->gameObject->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	}

	{
		gm::GMModel* sphere = nullptr;
		gm::GMPrimitiveCreator::createSphere(1.0f, 64, 64, &sphere);
		gm::GMShader& shader = sphere->getShader();
		shader.setIlluminationModel(gm::GMIlluminationModel::Phong);
		shader.getMaterial().ks = GMVec3(0.02f);
		shader.getMaterial().kd = GMVec3(0.05f);
		shader.getMaterial().shininess = 99;

		gm::ITexture* diffuse = nullptr;
		gm::GMToolUtil::createTexture("pbr/albedo.png", &diffuse);
		gm::ITexture* normal = nullptr;
		gm::GMToolUtil::createTexture("pbr/normal.png", &normal);

		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, diffuse);
		gm::GMToolUtil::addTextureToShader(shader, diffuse, gm::GMTextureType::Diffuse);

		getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, normal);
		gm::GMToolUtil::addTextureToShader(shader, normal, gm::GMTextureType::NormalMap);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Model, sphere);
		d->gameObject_Phong = new gm::GMGameObject(asset);
		d->gameObject_Phong->setTranslation(Translate(GMVec3(.66f, 0, .5f)));
		d->gameObject_Phong->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	}

	asDemoGameWorld(getDemoWorldReference())->addObject("pbr", d->gameObject);
	asDemoGameWorld(getDemoWorldReference())->addObject("phong", d->gameObject_Phong);
}

void Demo_Phong_PBR::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = GM.getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->draggingL = false;
		GM.getMainWindow()->setLockWindow(false);
	}
	if (state.downButton & gm::GMMouseButton_Right)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingR = true;
		GM.getMainWindow()->setLockWindow(true);
	}
	else if (state.upButton & gm::GMMouseButton_Right)
	{
		d->draggingR = false;
		GM.getMainWindow()->setLockWindow(false);
	}
}

void Demo_Phong_PBR::handleDragging()
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
		d->gameObject_Phong->setRotation(q);

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

void Demo_Phong_PBR::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* directLight = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Direct, &directLight);
			GM_ASSERT(directLight);
			gm::GMfloat lightPos[] = { 1, 1, -1 };
			directLight->setLightPosition(lightPos);
			gm::GMfloat color[] = { 20, 20, 20 };
			directLight->setLightColor(color);
			GM.getGraphicEngine()->addLight(directLight);
		}

		{
			gm::ILight* ambientLight = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Ambient, &ambientLight);
			GM_ASSERT(ambientLight);
			gm::GMfloat color[] = { .05f, .05f, .05f };
			ambientLight->setLightColor(color);
			GM.getGraphicEngine()->addLight(ambientLight);
		}
	}
}

void Demo_Phong_PBR::event(gm::GameMachineHandlerEvent evt)
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
	case gm::GameMachineHandlerEvent::Simulate:
		getDemoWorldReference()->simulateGameWorld();
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
	{
		handleMouseEvent();
		handleDragging();
		getDemoWorldReference()->notifyControls();

		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		if (kbState.keyTriggered('N'))
			switchNormal();
		if (kbState.keyTriggered('X'))
		{
			gm::GMRenderConfig& config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
			config.set(gm::GMRenderConfigs::HDR_Bool, !config.get(gm::GMRenderConfigs::HDR_Bool).toBool());
		}
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

void Demo_Phong_PBR::onDeactivate()
{
	Base::onDeactivate();
	gm::GMRenderConfig& config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	config.set(gm::GMRenderConfigs::HDR_Bool, false);
}