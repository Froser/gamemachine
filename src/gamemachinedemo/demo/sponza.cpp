#include "stdafx.h"
#include "sponza.h"
#include <gmgameobject.h>
#include <gmmodelreader.h>
#include <gmcontrols.h>
#include <gmimagebuffer.h>

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
		gm::GMTextureAsset cubeMapTex;
		GM.getFactory()->createTexture(context, &cubeMap, cubeMapTex);

		for (auto slice : slices)
		{
			gm::GM_delete(slice);
		}

		return new gm::GMCubeMapGameObject(cubeMapTex);
	}

}

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

	widget->addControl(d->cpu = gm::GMControlLabel::createControl(
		widget,
		L"CPU裁剪：开",
		GMVec4(1, 1, 1, 1),
		10,
		top,
		250,
		30,
		false
	));

	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"关闭CPU裁剪",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->sponza)
		{
			d->sponza->setCullOption(gm::GMGameObjectCullOption::None);
			d->cpu->setText(L"CPU裁剪：关");
		}
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"打开CPU裁剪",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->sponza)
		{
			d->sponza->setCullOption(gm::GMGameObjectCullOption::AABB);
			d->cpu->setText(L"CPU裁剪：开");
		}
	});

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
	d->sponza->setCullOption(gm::GMGameObjectCullOption::AABB);
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sponza", d->sponza);

	d->skyObject = createCubeMap(getDemoWorldReference()->getContext());
	d->skyObject->setScaling(Scale(GMVec3(1500, 1500, 1500)));
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sky", d->skyObject);
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
		if (d->sponza)
			d->sponza->update(GM.getRunningStates().lastFrameElpased);
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
			gm::GMfloat ambientIntensity[] = { .2f, .2f, .2f };
			gm::GMfloat diffuseIntensity[] = { 1.f, 1.f, 1.f };
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