#include "stdafx.h"
#include "customlight.h"
#include <linearmath.h>
#include <gmwidget.h>

namespace
{
	const gm::GMCameraLookAt s_lookAt = gm::GMCameraLookAt(
		GMVec3(0, 0, 1),
		GMVec3(0, 0, -1)
	);

	class GLSpotlight : public gm::ILight
	{
	public:
		virtual void activateLight(gm::GMuint32, gm::ITechnique*)
		{

		}

		virtual bool setLightAttribute3(GMLightAttribute attr, gm::GMfloat* value) override
		{
			switch (attr)
			{
			case Position:
			{
				this->position[0] = value[0];
				this->position[1] = value[1];
				this->position[2] = value[2];
				this->position[3] = 1.0f;
				break;
			}
			case ILight::Color:
			{
				this->color[0] = value[0];
				this->color[1] = value[1];
				this->color[2] = value[2];
				this->color[3] = 1.0f;
				break;
			}
			default:
				return false;
			}
			return true;
		}

	private:
		gm::GMfloat position[4];
		gm::GMfloat color[4];
	};
}

void Demo_CustomLight::init()
{
	D(d);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));

	// 创建一个带纹理的对象
	GMVec2 extents = GMVec2(1.f, .5f);
	gm::GMSceneAsset asset;
	gm::GMPrimitiveCreator::createQuadrangle(extents, 0, asset);
	
	gm::GMModel* model = asset.getScene()->getModels()[0].getModel();
	model->getShader().getMaterial().setDiffuse(GMVec3(1, 1, 1));
	model->getShader().getMaterial().setSpecular(GMVec3(0));

	gm::GMTextureAsset tex = gm::GMToolUtil::createTexture(getDemoWorldReference()->getContext(), "floor.png");
	gm::GMToolUtil::addTextureToShader(model->getShader(), tex, gm::GMTextureType::Diffuse);
	getDemoWorldReference()->getAssets().addAsset(tex);

	gm::GMAsset quadAsset = getDemoWorldReference()->getAssets().addAsset(asset);
	gm::GMGameObject* obj = new gm::GMGameObject(quadAsset);
	obj->setRotation(Rotate(Radians(80.f), GMVec3(1, 0, 0)));
	asDemoGameWorld(getDemoWorldReference())->addObject("texture", obj);

	gm::GMWidget* widget = createDefaultWidget();
	widget->setSize(widget->getSize().width, getClientAreaTop() + 40);
}

void Demo_CustomLight::event(gm::GameMachineHandlerEvent evt)
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

void Demo_CustomLight::setLookAt()
{
	D(d);
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	camera.lookAt(s_lookAt);
}

void Demo_CustomLight::setDefaultLights()
{
	D(d);
	if (isInited())
	{
		if (GM.getRunningStates().renderEnvironment == gm::GMRenderEnvironment::OpenGL)
		{
			gm::ILight* light = new GLSpotlight();
			gm::GMfloat lightPos[] = { 0, 0, -.2f };
			light->setLightAttribute3(gm::ILight::Position, lightPos);
			gm::GMfloat color[] = { .7f, .7f, .7f };
			light->setLightAttribute3(gm::ILight::Color, color);
			d->engine->addLight(light);
		}
	}
}
