#include "stdafx.h"
#include "scene_control.h"
#include <gmutilities.h>
#include "handler.h"
#include <gmlight.h>
#include "util.h"
#include "scene_model.h"

namespace
{
	void setLightAttributes(ILight* light, const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity)
	{
		GM_ASSERT(light);
		light->setLightAttribute3(GMLight::Position, ValuePointer(position));
		light->setLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(diffuseIntensity));
		light->setLightAttribute3(GMLight::AmbientIntensity, ValuePointer(ambientIntensity));
	}

	GMCamera defaultCamera()
	{
		static std::once_flag s_flag;
		static GMCamera s_camera;
		std::call_once(s_flag, [](GMCamera&){
			s_camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);
			GMCameraLookAt lookAt;
			lookAt.lookAt = { 0, 0, 1 };
			lookAt.position = { 0, 0, -1 };
			s_camera.lookAt(lookAt);
		}, s_camera);
		return s_camera;
	}
}

namespace core
{
	SceneControl::SceneControl(Handler* handler, QObject* parent /*= nullptr*/)
		: QObject(parent)
		, m_handler(handler)
	{
		setDefaultLight(GMVec3(0, 0, -.2f), GMVec3(.7f, .7f, .7f), GMVec3(0, 0, 0));
		setViewCamera(defaultCamera());
	}

	void SceneControl::onSceneModelCreated(SceneModel* model)
	{
		resetModel(model);
		clearRenderList();
		renderPlain();
	}

	void SceneControl::setViewCamera(const GMCamera& camera)
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->setCamera(camera);
	}

	void SceneControl::setDefaultColor(const GMVec4& color)
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->getDefaultFramebuffers()->setClearColor(ValuePointer(color));
	}

	void SceneControl::setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity)
	{
		auto engine = m_handler->getContext()->getEngine();
		if (m_lights.defaultLightIndex == -1)
		{
			ILight* light = nullptr;
			GM.getFactory()->createLight(GMLightType::PointLight, &light);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			m_lights.defaultLightIndex = engine->addLight(light);
		}
		else
		{
			ILight* light = engine->getLight(m_lights.defaultLightIndex);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			engine->update(GMUpdateDataType::LightChanged);
		}
	}

	void SceneControl::clearRenderList()
	{
		m_handler->getWorld()->clearRenderList();
	}

	void SceneControl::renderLogo()
	{
		setViewCamera(defaultCamera());
		if (m_assets.logo.asset.isEmpty())
		{
			m_assets.logo.asset = createLogo();
			m_assets.logo.object = new GMGameObject(m_assets.logo.asset);
			m_handler->getWorld()->addObjectAndInit(m_assets.logo.object);
		}
		m_handler->getWorld()->addToRenderList(m_assets.logo.object);
		emit renderUpdate();
	}

	void SceneControl::renderPlain()
	{
		if (m_assets.plain.asset.isEmpty())
		{
			// 创建一个平面
			GMPlainDescription desc = {
				-128.f,
				0,
				-128.f,
				256.f,
				256,
				50,
				50,
				{ 1, 1, 1 }
			};

			utCreatePlain(desc, m_assets.plain.asset);
			m_assets.plain.object = new GMGameObject(m_assets.plain.asset);
			m_handler->getWorld()->addObjectAndInit(m_assets.plain.object);
		}
		m_handler->getWorld()->addToRenderList(m_assets.plain.object);
		emit renderUpdate();
	}

	void SceneControl::resetModel(SceneModel* model)
	{
		// 负责重置Model，更新渲染，更新资源
		m_model = model;
		
		// 重新生成场景相关的资源:
		// 重置摄像机
		m_sceneViewCamera.setPerspective(Radian(75.f), .75f, .1f, 1000); //TODO aspect需要计算，near和far需要从全局拿
		GlobalProperties& props = model->getProperties();
		GMCameraLookAt lookAt = GMCameraLookAt::makeLookAt(
			GMVec3(props.viewCamera.posX, props.viewCamera.posY, props.viewCamera.posZ),
			GMVec3(props.viewCamera.lookAtX, props.viewCamera.lookAtY, props.viewCamera.lookAtZ));
		m_sceneViewCamera.lookAt(lookAt);
		setViewCamera(m_sceneViewCamera);
	}

	GMSceneAsset SceneControl::createLogo()
	{
		// 创建一个带纹理的对象
		GMVec2 extents = GMVec2(1.f, .5f);
		GMSceneAsset asset;
		GMPrimitiveCreator::createQuadrangle(extents, 0, asset);

		GMModel* model = asset.getScene()->getModels()[0].getModel();
		model->getShader().getMaterial().setDiffuse(GMVec3(1, 1, 1));
		model->getShader().getMaterial().setSpecular(GMVec3(0));

		GMTextureAsset tex = GMToolUtil::createTexture(m_handler->getContext(), "gamemachine.png"); //TODO 考虑从qrc拿
		GMToolUtil::addTextureToShader(model->getShader(), tex, GMTextureType::Diffuse);
		return asset;
	}
}
