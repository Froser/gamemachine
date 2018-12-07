#include "stdafx.h"
#include "light.h"
#include <linearmath.h>
#include <gmwidget.h>

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, 0, 1),
		GMVec3(0, 0, -1)
	);
}

void Demo_Light::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

	// 创建一个带纹理的对象
	GMVec2 extents = GMVec2(1.f, .5f);
	gm::GMSceneAsset asset;
	gm::GMPrimitiveCreator::createQuadrangle(extents, 0, asset);

	onAssetAdded(asset);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(asset);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	
	onGameObjectAdded(obj);

	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);
}

void Demo_Light::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
}

void Demo_Light::onAssetAdded(gm::GMModelAsset asset)
{
	gm::GMModel* model = asset.getScene()->getModels()[0].getModel();
	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "floor.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);
}

void Demo_Light::onGameObjectAdded(gm::GMGameObject* obj)
{
	obj->setRotation(Rotate(Radians(80.f), GMVec3(1, 0, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);
}

void Demo_Light::event(gm::GameMachineHandlerEvent evt)
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
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Demo_Light_Point::setDefaultLights()
{
	gm::ILight* light = nullptr;
	GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
	GM_ASSERT(light);
	gm::GMfloat lightPos[] = { 0, 2, 0 };
	light->setLightAttribute3(gm::GMLight::Position, lightPos);

	gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
	light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

	gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
	light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
	light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);
	getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
}

//////////////////////////////////////////////////////////////////////////

void Demo_Light_Point_NormalMap::onGameObjectAdded(gm::GMGameObject* obj)
{
	obj->setRotation(Rotate(Radians(20.f), GMVec3(1, 0, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);
}

void Demo_Light_Point_NormalMap::setDefaultLights()
{
	gm::ILight* light = nullptr;
	GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
	GM_ASSERT(light);
	gm::GMfloat lightPos[] = { 0, 1, -1 };
	light->setLightAttribute3(gm::GMLight::Position, lightPos);

	gm::GMfloat ambientIntensity[] = { 0, 0, 0 };
	light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

	gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
	light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
	light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);
	getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
}

//////////////////////////////////////////////////////////////////////////
void Demo_Light_Directional_Normalmap::setDefaultLights()
{
	D(d);
	gm::ILight* light = nullptr;
	{
		GM.getFactory()->createLight(gm::GMLightType::DirectionalLight, &light);
		GM_ASSERT(light);
		gm::GMfloat lightPos[] = { 0, 0, -1 };
		light->setLightAttribute3(gm::GMLight::Position, lightPos);

		gm::GMfloat ambientIntensity[] = { 0, 0, 0 };
		light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

		gm::GMfloat diffuseIntensity[] = { .4f, .4f, .4f };
		light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
		light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);

		gm::GMfloat lightDirection[] = { 0, 0, 1 };
		light->setLightAttribute3(gm::GMLight::Direction, lightDirection);
		d->light = getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
	}

	{
		GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
		GM_ASSERT(light);

		gm::GMfloat ambientIntensity[] = { .1f, .1f, .1f };
		light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
		d->ambientLight = getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
	}
}

void Demo_Light_Directional_Normalmap::onGameObjectAdded(gm::GMGameObject* obj)
{
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);
}

//////////////////////////////////////////////////////////////////////////
void Demo_Light_Directional::setDefaultLights()
{
	gm::ILight* light = nullptr;
	GM.getFactory()->createLight(gm::GMLightType::DirectionalLight, &light);
	GM_ASSERT(light);
	gm::GMfloat lightPos[] = { 0, 2, 0 };
	light->setLightAttribute3(gm::GMLight::Position, lightPos);

	gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
	light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

	gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
	light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
	light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);

	gm::GMfloat lightDirection[] = { 2, -1, 0 };
	light->setLightAttribute3(gm::GMLight::Direction, lightDirection);
	getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
}

void Demo_Light_Point_NormalMap::onAssetAdded(gm::GMModelAsset asset)
{
	gm::GMModel* model = asset.getScene()->getModels()[0].getModel();
	model->getShader().setCull(gm::GMS_Cull::Cull);
	model->getShader().getMaterial().setDiffuse(GMVec3(.6f, .2f, .3f));
	model->getShader().getMaterial().setSpecular(GMVec3(.1f, .2f, .3f));
	model->getShader().getMaterial().setAmbient(GMVec3(1, 1, 1));
	model->getShader().getMaterial().setShininess(20);
	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "bnp.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::NormalMap);
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);
}

void Demo_Light_Directional_Normalmap::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Update:
	{
		auto im = getDemonstrationWorld()->getContext()->getWindow()->getInputMananger();
		gm::IWindow* window = getDemonstrationWorld()->getContext()->getWindow();
		const auto& rc = window->getRenderRect();
		auto hw = rc.width / 2, hh = rc.height / 2;

		auto ms = im->getMouseState().mouseState();
		gm::GMfloat x = ((gm::GMfloat)ms.posX / rc.width) - .5f;
		gm::GMfloat y = .5f - ((gm::GMfloat)ms.posY / rc.width);
		GMVec3 lightDirection = Normalize(GMVec3(x, y, 1));
		gm::ILight* light = getDemonstrationWorld()->getContext()->getEngine()->getLight(d->light);
		if (light)
		{
			gm::GMfloat direction[] = { lightDirection.getX(), lightDirection.getY(), lightDirection.getZ() };
			light->setLightAttribute3(gm::GMLight::Direction, direction);
			getDemonstrationWorld()->getContext()->getEngine()->update(gm::GMUpdateDataType::LightChanged); // Don't forget this !!!
		}
	}
	}
}

void Demo_Light_Spotlight::setDefaultLights()
{
	gm::ILight* light = nullptr;
	GM.getFactory()->createLight(gm::GMLightType::Spotlight, &light);
	GM_ASSERT(light);
	gm::GMfloat lightPos[] = { 0, 2, 0 };
	light->setLightAttribute3(gm::GMLight::Position, lightPos);

	gm::GMfloat ambientIntensity[] = { .8f, .8f, .8f };
	light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

	gm::GMfloat diffuseIntensity[] = { .7f, .7f, .7f };
	light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
	light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);

	gm::GMfloat lightDirection[] = { 0, -1, 0 };
	light->setLightAttribute3(gm::GMLight::Direction, lightDirection);
	light->setLightAttribute(gm::GMLight::CutOff, 10.f);
	getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
}

void Demo_Light_Spotlight_Normalmap::setDefaultLights()
{
	D(d);
	gm::ILight* light = nullptr;
	{
		GM.getFactory()->createLight(gm::GMLightType::Spotlight, &light);
		GM_ASSERT(light);
		gm::GMfloat lightPos[] = { 0, 0, -1 };
		light->setLightAttribute3(gm::GMLight::Position, lightPos);

		gm::GMfloat ambientIntensity[] = { .5f, .5f, .5f };
		light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);

		gm::GMfloat diffuseIntensity[] = { .4f, .4f, .4f };
		light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
		light->setLightAttribute(gm::GMLight::AttenuationLinear, .1f);

		gm::GMfloat lightDirection[] = { 0, 0, 1 };
		light->setLightAttribute3(gm::GMLight::Direction, lightDirection);
		light->setLightAttribute(gm::GMLight::CutOff, 15.f);
		d->light = getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
	}

	{
		GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
		GM_ASSERT(light);

		gm::GMfloat ambientIntensity[] = { .1f, .1f, .1f };
		light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
		d->ambientLight = getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
	}
}