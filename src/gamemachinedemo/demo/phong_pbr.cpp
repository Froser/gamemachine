#include "stdafx.h"
#include "phong_pbr.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>
#include <gmcontrols.h>

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
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_Phong_PBR::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	{
		gm::GMModelAsset sphere;
		gm::GMPrimitiveCreator::createSphere(1.0f, 64, 64, sphere);
		gm::GMShader& shader = sphere.getModel()->getShader();
		shader.setIlluminationModel(gm::GMIlluminationModel::CookTorranceBRDF);
		gm::GMTextureAsset albedo;
		gm::GMTextureAsset metallicRoughnessAO;
		gm::GMTextureAsset normal;
		gm::GMToolUtil::createPBRTextures(
			getDemoWorldReference()->getContext(),
			"pbr/albedo.png",
			"pbr/metallic.png",
			"pbr/roughness.png",
			"",
			"pbr/normal.png",
			albedo,
			metallicRoughnessAO,
			normal
		);

		getDemoWorldReference()->getAssets().addAsset(albedo);
		gm::GMToolUtil::addTextureToShader(shader, albedo, gm::GMTextureType::Albedo);

		getDemoWorldReference()->getAssets().addAsset(normal);
		gm::GMToolUtil::addTextureToShader(shader, normal, gm::GMTextureType::NormalMap);

		getDemoWorldReference()->getAssets().addAsset(metallicRoughnessAO);
		gm::GMToolUtil::addTextureToShader(shader, metallicRoughnessAO, gm::GMTextureType::MetallicRoughnessAO);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(sphere);
		d->gameObject = new gm::GMGameObject(asset);
		d->gameObject->setTranslation(Translate(GMVec3(-.66f, 0, .5f)));
		d->gameObject->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	}

	{
		gm::GMModelAsset sphere;
		gm::GMPrimitiveCreator::createSphere(1.0f, 64, 64, sphere);
		gm::GMShader& shader = sphere.getModel()->getShader();
		shader.setIlluminationModel(gm::GMIlluminationModel::Phong);
		shader.getMaterial().ks = GMVec3(0.02f);
		shader.getMaterial().kd = GMVec3(0.05f);
		shader.getMaterial().shininess = 99;

		gm::GMTextureAsset albedo = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "pbr/albedo.png");
		gm::GMTextureAsset normal = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "pbr/normal.png");

		getDemoWorldReference()->getAssets().addAsset(albedo);
		gm::GMToolUtil::addTextureToShader(shader, albedo, gm::GMTextureType::Diffuse);

		getDemoWorldReference()->getAssets().addAsset(normal);
		gm::GMToolUtil::addTextureToShader(shader, normal, gm::GMTextureType::NormalMap);

		gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(sphere);
		d->gameObject_Phong = new gm::GMGameObject(asset);
		d->gameObject_Phong->setTranslation(Translate(GMVec3(.66f, 0, .5f)));
		d->gameObject_Phong->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	}

	asDemoGameWorld(getDemoWorldReference())->addObject("pbr", d->gameObject);
	asDemoGameWorld(getDemoWorldReference())->addObject("phong", d->gameObject_Phong);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	gm::GMControlButton* button = nullptr;
	widget->addButton(
		L"开/关HDR",
		10,
		top,
		250,
		30,
		false,
		&button
	);

	connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		gm::GMRenderConfig config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
		config.set(gm::GMRenderConfigs::HDR_Bool, !config.get(gm::GMRenderConfigs::HDR_Bool).toBool());
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Phong_PBR::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
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

void Demo_Phong_PBR::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

	if (d->draggingL)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;

		GMQuat q = Rotate(d->gameObject->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
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
			getDemonstrationWorld()->getContext()->getEngine()->addLight(directLight);
		}

		{
			gm::ILight* ambientLight = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Ambient, &ambientLight);
			GM_ASSERT(ambientLight);
			gm::GMfloat color[] = { .05f, .05f, .05f };
			ambientLight->setLightColor(color);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(ambientLight);
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

		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
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