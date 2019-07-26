#include "stdafx.h"
#include "csm.h"
#include <gmwidget.h>
#include <gmcontrols.h>
#include <gmmodelreader.h>
#include <gmgraphicengine.h>

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt::makeLookAt(
		GMVec3(100.0f, 15.0f, 5.0f),
		GMVec3(0, 0, 0)
	);
}

void Demo_CSM::onDeactivate()
{
	D(d);
	D_BASE(db, Base);
	db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);
	DemoHandler::onDeactivate();
}

void Demo_CSM::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(PI / 4, 1.33333f, .05f, 300);
	camera.lookAt(s_lookAt);
	d->viewerCamera = camera;
	d->lookAtRotation = Identity<GMQuat>();
}


Demo_CSM::Demo_CSM(DemonstrationWorld* parentDemonstrationWorld)
	: Base(parentDemonstrationWorld)
{
	GM_CREATE_DATA();
}

void Demo_CSM::init()
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
		"cat/cat.obj",
		getDemoWorldReference()->getContext()
	);

	gm::GMAsset models;
	gm::GMModelReader::load(loadSettings, models);

	gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(models);

	d->gameObject = new gm::GMGameObject(asset);
	d->gameObject->setTranslation(Translate(GMVec3(50.f, 5.f, 0)));
	d->gameObject->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	d->gameObject->setRotation(Rotate(PI, GMVec3(0, 1, 0)));

	asDemoGameWorld(getDemoWorldReference())->addObject("cat", d->gameObject);

	d->gameObject2 = new gm::GMGameObject(asset);
	d->gameObject2->setTranslation(Translate(GMVec3(25.f, 5.f, 0.f)));
	d->gameObject2->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	d->gameObject2->setRotation(Rotate(PI, GMVec3(0, 1, 0)));

	asDemoGameWorld(getDemoWorldReference())->addObject("cat2", d->gameObject2);

	d->gameObject3 = new gm::GMGameObject(asset);
	d->gameObject3->setTranslation(Translate(GMVec3(0.f, 5.f, 0.f)));
	d->gameObject3->setScaling(Scale(GMVec3(.5f, .5f, .5f)));
	d->gameObject3->setRotation(Rotate(PI, GMVec3(0, 1, 0)));

	asDemoGameWorld(getDemoWorldReference())->addObject("cat3", d->gameObject3);

	// 创建地板
	gm::GMGameObject* ground = new gm::GMGameObject();
	ground->setTranslation(Translate(GMVec3(0, -50, 0)));

	gm::GMSceneAsset groundShapeAsset;
	gm::GMPrimitiveCreator::createCube(GMVec3(500, 50, 500), groundShapeAsset);
	groundShapeAsset.getScene()->getModels()[0].getModel()->getShader().getMaterial().setAmbient(GMVec3(.8125f / .7f, .644f / .7f, .043f / .7f));
	groundShapeAsset.getScene()->getModels()[0].getModel()->getShader().getMaterial().setDiffuse(GMVec3(.1f));
	groundShapeAsset.getScene()->getModels()[0].getModel()->getShader().getMaterial().setSpecular(GMVec3(.4f));
	groundShapeAsset.getScene()->getModels()[0].getModel()->getShader().getMaterial().setShininess(9);
	ground->setAsset(getDemoWorldReference()->getAssets().addAsset(groundShapeAsset));

	// add to world
	asDemoGameWorld(getDemoWorldReference())->addObject(L"ground", ground);

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"切换可视化CSM范围",
		10,
		top,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		// Visualize CSM
		if (d->view == CameraView)
		{
			gm::GMRenderConfig config = getDemonstrationWorld()->getContext()->getEngine()->getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
			config.set(gm::GMRenderConfigs::ViewCascade_Bool, d->viewCSM = !config.get(gm::GMRenderConfigs::ViewCascade_Bool).toBool());
		}
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"角色视图",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->view = CameraView;

		gm::GMRenderConfig config = getDemonstrationWorld()->getContext()->getEngine()->getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
		config.set(gm::GMRenderConfigs::ViewCascade_Bool, d->viewCSM);
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"阴影视图",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->view = ShadowCameraView;

		gm::GMRenderConfig config = getDemonstrationWorld()->getContext()->getEngine()->getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
		config.set(gm::GMRenderConfigs::ViewCascade_Bool, false);
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_CSM::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	
	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->dragging = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->dragging = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}

void Demo_CSM::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputManager()->getMouseState();
	gm::GMMouseState state = ms.state();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
	if (d->dragging && d->view == CameraView)
	{
		gm::GMfloat rotateX = d->mouseDownX - state.posX;
		gm::GMfloat rotateY = d->mouseDownY - state.posY;
		GMVec3 lookDir3 = Normalize(s_lookAt.lookDirection);
		GMVec4 lookAt = GMVec4(lookDir3, 1.f);
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
		d->viewerCamera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	}
}


void Demo_CSM::updateCamera()
{
	D(d);
	if (d->view == CameraView)
	{
		getDemonstrationWorld()->getContext()->getEngine()->setCamera(d->viewerCamera);
	}
	else
	{
		GM_ASSERT(d->view = ShadowCameraView);
		getDemonstrationWorld()->getContext()->getEngine()->setCamera(d->shadowCamera);
	}
}

void Demo_CSM::setDefaultLights()
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

			gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

			gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}

		// 设置CSM
		{
			const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
			gm::GMShadowSourceDesc desc;
			desc.position = GMVec4(-3.f, 3.f, -3.f, 1);
			desc.type = gm::GMShadowSourceDesc::CSMShadow;

			gm::GMCamera shadowCamera;
			shadowCamera.setPerspective(PI / 4, 1.f, 1, 1000);

			desc.camera = shadowCamera;
			desc.biasMax = desc.biasMin = 0.005f;
			desc.width = windowStates.renderRect.width;
			desc.height = windowStates.renderRect.height;
			desc.cascades = 3;
			desc.cascadePartitions = { .2f, .3f, 1 };

			gm::GMCameraLookAt lookAt = gm::GMCameraLookAt::makeLookAt(
				GMVec3(-160.0f, 150.0f, -110.3f),
				GMVec3(0, 0, 0)
			);
			desc.camera.lookAt(lookAt);
			d->shadowCamera = desc.camera;

			getDemonstrationWorld()->getContext()->getEngine()->setShadowSource(desc);
		}
	}
}

void Demo_CSM::event(gm::GameMachineHandlerEvent evt)
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
		updateCamera();
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
