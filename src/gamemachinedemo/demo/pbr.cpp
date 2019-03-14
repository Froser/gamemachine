#include "stdafx.h"
#include "pbr.h"
#include <linearmath.h>
#include <gmmodelreader.h>
#include <gmimagebuffer.h>
#include <gmcontrols.h>
#include <gmwidget.h>

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, 0, 1),
		GMVec3(0, 0, -1)
	);
}

void Demo_PBR::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
	d->lookAtRotation = Identity<GMQuat>();
}

void Demo_PBR::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	{
		gm::GMSceneAsset sphere;
		gm::GMPrimitiveCreator::createSphere(1.0f, 64, 64, sphere);

		gm::GMShader& shader = sphere.getScene()->getModels()[0].getModel()->getShader();
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
		d->gameObject->setTranslation(Translate(GMVec3(0, 0, 0)));
		d->gameObject->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	}

	asDemoGameWorld(getDemoWorldReference())->addObject("sphere", d->gameObject);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"开/关HDR",
		10,
		top,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		gm::GMRenderConfig config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
		config.set(gm::GMRenderConfigs::HDR_Bool, !config.get(gm::GMRenderConfigs::HDR_Bool).toBool());
	});
	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_PBR::handleMouseEvent()
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

void Demo_PBR::handleDragging()
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

void Demo_PBR::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat lightPos[] = { 1, 1, -1 };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);

			gm::GMfloat ambientIntensity[] = { .05f, .05f, .05f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

			gm::GMfloat diffuseIntensity[] = { 20, 20, 20 };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_PBR::event(gm::GameMachineHandlerEvent evt)
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

		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
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