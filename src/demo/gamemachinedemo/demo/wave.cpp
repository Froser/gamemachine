#include "stdafx.h"
#include "wave.h"
#include <gmgameobject.h>
#include <gmimagebuffer.h>
#include <gmcontrols.h>
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	gm::GMCubeMapGameObject* createCubeMap(const gm::IRenderContext* context)
	{
		gm::GMGamePackage* pk = GM.getGamePackageManager();
		gm::GMImage* slices[6] = { nullptr };
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posx.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[0]);
			GM_ASSERT(slices[0]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negx.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[1]);
			GM_ASSERT(slices[1]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posy.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[2]);
			GM_ASSERT(slices[2]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negy.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[3]);
			GM_ASSERT(slices[3]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posz.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[4]);
			GM_ASSERT(slices[4]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negz.jpg", &buf);
			gm::GMImageReader::load(buf.getData(), buf.getSize(), &slices[5]);
			GM_ASSERT(slices[5]);
		}

		gm::GMCubeMapBuffer cubeMap(*slices[0], *slices[1], *slices[2], *slices[3], *slices[4], *slices[5]);
		gm::GMTextureAsset cubeMapTex;
		GM.getFactory()->createTexture(context, &cubeMap, cubeMapTex);

		for (auto slice : slices)
		{
			gm::GM_delete(slice);
		}

		return new gm::GMCubeMapGameObject(cubeMapTex);
	}

}

void Demo_Wave::init()
{
	Base::init();
	createMenu();
	createObject();
}

void Demo_Wave::createMenu()
{
	D(d);
	D_BASE(db, DemoHandler);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"停止水波",
		10,
		top,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->wave)
			d->wave->pause();
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"继续水波",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->wave)
			d->wave->play();
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"停止水流",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		setWaterFlow(false);
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"开始水流",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		setWaterFlow(true);
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"使用CPU/GPU渲染",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		auto acc = d->wave->getHandwareAcceleration();
		d->wave->setHandwareAcceleration(acc == gm::GMWaveGameObjectHardwareAcceleration::GPU ? gm::GMWaveGameObjectHardwareAcceleration::CPU : gm::GMWaveGameObjectHardwareAcceleration::GPU);
	});

	widget->addControl(d->handwareAccelerationLabel = gm::GMControlLabel::createControl(
		widget,
		gm::GMString(),
		GMVec4(1, 1, 1, 1),
		10,
		top += 40,
		250,
		30,
		false
	));

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Wave::createObject()
{
	D(d);
	D_BASE(db, Base);
	gm::GMWaveGameObjectDescription desc = {
		-256.f,
		-256.f,
		512.f,
		512.f,
		80.f,
		50,
		50,
		100,
		100,
		2,
		2,
	};

	gm::GMWaveGameObject* wave = gm::GMWaveGameObject::create(desc);
	GMVec3 direction1 = Normalize(GMVec3(1, 0, 1));
	GMVec3 direction2 = Normalize(GMVec3(-1, 0, 1));
	GMVec3 direction3 = Normalize(GMVec3(-1, 0, 0));
	Vector<gm::GMWaveDescription> wd = {
		{ 0.f, 1.25f, direction1.getX(), direction1.getY(), direction1.getZ(), 2.f, 3.f },
		{ 0.03f, .5f, direction2.getX(), direction2.getY(), direction2.getZ(), 3.f, 1.5f },
		//{ 0.03f, 1.5f, direction3, 5.f, 3.f },
	};
	wave->setWaveDescriptions(wd);
	wave->play();
	d->wave = wave;
	gm::GMModel* waveModel = d->wave->getModel();
	gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(db->parentDemonstrationWorld->getContext(), L"water.tga");
	gm::GMToolUtil::addTextureToShader(waveModel->getShader(), texture, gm::GMTextureType::Ambient);
	gm::GMToolUtil::addTextureToShader(waveModel->getShader(), texture, gm::GMTextureType::Diffuse);
	gm::GMToolUtil::addTextureToShader(waveModel->getShader(), texture, gm::GMTextureType::Specular);

	texture = gm::GMToolUtil::createTexture(db->parentDemonstrationWorld->getContext(), L"water-normal.tga");
	gm::GMToolUtil::addTextureToShader(waveModel->getShader(), texture, gm::GMTextureType::NormalMap);

	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapS(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapT(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Diffuse).setWrapS(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Diffuse).setWrapT(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Specular).setWrapS(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Specular).setWrapT(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::NormalMap).setWrapS(gm::GMS_Wrap::MirroredRepeat);
	waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::NormalMap).setWrapT(gm::GMS_Wrap::MirroredRepeat);

	setWaterFlow(true);
	waveModel->getShader().getMaterial().setAmbient(GMVec3(.4f, .4f, .4f));
	waveModel->getShader().getMaterial().setDiffuse(GMVec3(.6f, .6f, .6f));
	waveModel->getShader().getMaterial().setSpecular(GMVec3(.6f, .6f, .6f));
	waveModel->getShader().getMaterial().setShininess(50);

	asDemoGameWorld(getDemoWorldReference())->addObject(L"wave", d->wave);

	d->skyObject = createCubeMap(getDemoWorldReference()->getContext());
	d->skyObject->setScaling(Scale(GMVec3(1000, 1000, 1000)));
	d->skyObject->setTranslation(Translate(GMVec3(0, 20, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sky", d->skyObject);
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
		if (d->wave)
			d->wave->update(GM.getRunningStates().lastFrameElapsed);
		if (d->handwareAccelerationLabel)
			d->handwareAccelerationLabel->setText(d->wave->getHandwareAcceleration() == gm::GMWaveGameObjectHardwareAcceleration::CPU ? L"渲染方式：CPU" : L"渲染方式：GPU");
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

void Demo_Wave::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookDirection = Normalize(GMVec3(0, -.5f, 1));
	lookAt.position = GMVec3(0, 10, 0);
	camera.lookAt(lookAt);

	D(d);
	d->cameraUtility.setCamera(&camera);
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

void Demo_Wave::onActivate()
{
	D(d);
	d->activate = true;
	setMouseTrace(true);
	Base::onActivate();
}

void Demo_Wave::onDeactivate()
{
	D(d);
	d->activate = false;
	setMouseTrace(false);
	Base::onDeactivate();
}

void Demo_Wave::setMouseTrace(bool enabled)
{
	D(d);
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseTrace = enabled;

	// 鼠标跟踪开启时，detecting mode也开启，每一帧将返回窗口中心，以获取鼠标移动偏量
	mouseState.setDetectingMode(d->mouseTrace);
}

void Demo_Wave::setWaterFlow(bool flow)
{
	D(d);
	if (d->wave)
	{
		gm::GMModel* waveModel = d->wave->getModel();
		if (flow)
		{
			gm::GMS_TextureTransform tt;
			tt.type = gm::GMS_TextureTransformType::Scroll;
			tt.p1 = .025f;
			tt.p2 = .05f;
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Diffuse).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Specular).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::NormalMap).setTextureTransform(0, tt);
		}
		else
		{

			gm::GMS_TextureTransform tt;
			tt.type = gm::GMS_TextureTransformType::NoTextureTransform;
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Diffuse).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Specular).setTextureTransform(0, tt);
			waveModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::NormalMap).setTextureTransform(0, tt);
		}
	}
}