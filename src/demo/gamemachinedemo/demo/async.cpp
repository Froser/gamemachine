#include "stdafx.h"
#include "async.h"
#include <gmm.h>
#include <gmmodelreader.h>

void Demo_Async::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	GM_ASSERT(!getDemoWorldReference());
	const gm::IRenderContext* context = db->parentDemonstrationWorld->getContext();
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(context));

	gm::GMTextGameObject* literature = new gm::GMTextGameObject(getDemoWorldReference()->getContext()->getWindow()->getRenderRect());
	gm::GMRect rect = { 0, 0, 300, 190 };
	gm::GMRect renderRc = context->getWindow()->getRenderRect();
	rect.x = (renderRc.width - rect.width) / 2;
	rect.y = (renderRc.height - rect.height) / 2;

	literature->setLineSpacing(10);
	literature->setGeometry(rect);
	literature->setText(L"资源正在读取中，请稍候...");
	gm::GMDemoGameWorld* world = gm::gm_cast<gm::GMDemoGameWorld*>(getDemoWorldReference().get());
	world->addObject("text", literature);

	d->future = gm::GMAsync::async(gm::GMAsync::Async, [this, d, context, literature]() {
		GM_CHILD_THREAD_RENDER(context->getWindow());
		gm::GMGamePackage& pk = *GM.getGamePackageManager();
		gm::GMModelLoadSettings loadSettings(
			"dragon/dragon.obj",
			context
		);

		gm::GMAsset models;
		bool b = gm::GMModelReader::load(loadSettings, models);
		gm::GMGameObject* obj = new gm::GMGameObject(models);

		mainThreadInvoke([literature, obj, this]() {
			// 加入容器
			obj->setScaling(Scale(GMVec3(.05f, .05f, .05f)));
			obj->setTranslation(Translate(GMVec3(0, -.2f, 0)));
			getDemoWorldReference()->addObjectAndInit(obj);
			getDemoWorldReference()->addToRenderList(obj);
			literature->getModel()->getShader().setVisible(false);
			setDefaultLights();
		});
	});

	createDefaultWidget();
}

void Demo_Async::setDefaultLights()
{
	D_BASE(db, Base);
	const gm::IRenderContext* context = db->parentDemonstrationWorld->getContext();

	gm::ILight* light = nullptr;
	GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
	gm::GMfloat ambientIntensity[] = { .1f, .1f, .1f, 1 };
	light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

	gm::GMfloat diffuseIntensity[] = { 2, 2, 2 };
	light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);

	gm::GMfloat lightPos[] = { -5, 5, -5 };
	light->setLightAttribute3(gm::GMLight::Position, lightPos);

	context->getEngine()->addLight(light);
}

void Demo_Async::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		invokeThreadFunctions();
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
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		if (d->future.valid())
			d->future.wait();
		break;
	default:
		break;
	}
}

void Demo_Async::mainThreadInvoke(std::function<void()> function)
{
	D(d);
	d->funcStack.push(function);
}

void Demo_Async::invokeThreadFunctions()
{
	D(d);
	if (!d->funcStack.empty())
	{
		std::function<void()> func = d->funcStack.top();
		d->funcStack.pop();
		func();
	}
}
