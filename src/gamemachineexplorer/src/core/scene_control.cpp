#include "stdafx.h"
#include "scene_control.h"
#include <gmutilities.h>
#include <gmdiscretedynamicsworld.h>
#include <gmphysicsshape.h>
#include <gmlight.h>
#include "handler.h"
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
			lookAt.lookDirection = { 0, 0, 1 };
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

	void SceneControl::onWidgetMousePress(shell::View* v, QMouseEvent* e)
	{
		m_mouseDownPos = e->pos();
		Asset selected = hitTest(m_mouseDownPos.x(), m_mouseDownPos.y());
		
		if (selected == m_assets[AT_Plane])
		{
			// 地板不能和其它元素同时选中
			m_selectedAssets.clear();
			if (!m_selectedAssets.contains(selected))
				m_selectedAssets.append(selected);
		}
		else
		{
			// 如果用户按住Ctrl，则添加到选择列表
			if (!(e->modifiers() & Qt::ControlModifier))
				m_selectedAssets.clear();
			if (!m_selectedAssets.contains(selected))
				m_selectedAssets.append(selected);
		}

		m_mouseDown = true;
	}

	void SceneControl::onWidgetMouseRelease(shell::View* v, QMouseEvent* e)
	{
		m_mouseDown = false;
	}

	void SceneControl::onWidgetMouseMove(shell::View*, QMouseEvent* e)
	{
		if (m_mouseDown)
		{
			// 先获取偏移
			float dx = e->pos().x() - m_mouseDownPos.x();
			float dy = e->pos().y() - m_mouseDownPos.y();
			
			// 如果选中了地板，移动镜头
			if (m_selectedAssets.contains(m_assets[AT_Plane]))
			{
				// 镜头位置移动
				GMCameraLookAt lookAt = m_sceneViewCamera.getLookAt();
				lookAt.position.setX(lookAt.position.getX() + dx);
				lookAt.position.setZ(lookAt.position.getZ() + dy);
				m_sceneViewCamera.setLookAt(lookAt);
				setViewCamera(m_sceneViewCamera);
				m_mouseDownPos = e->pos();
				emit renderUpdate();
			}
		}
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
		if (m_defaultLight.defaultLightIndex == -1)
		{
			ILight* light = nullptr;
			GM.getFactory()->createLight(GMLightType::PointLight, &light);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			m_defaultLight.defaultLightIndex = engine->addLight(light);
		}
		else
		{
			ILight* light = engine->getLight(m_defaultLight.defaultLightIndex);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			engine->update(GMUpdateDataType::LightChanged);
		}
	}

	const GMCamera& SceneControl::currentCamera()
	{
		auto engine = m_handler->getContext()->getEngine();
		return engine->getCamera();
	}

	void SceneControl::clearRenderList()
	{
		m_handler->getWorld()->clearRenderList();
	}

	void SceneControl::renderLogo()
	{
		setViewCamera(defaultCamera());
		if (m_assets[AT_Logo].asset.isEmpty())
		{
			m_assets[AT_Logo].asset = createLogo();
			m_assets[AT_Logo].object = new GMGameObject(m_assets[AT_Logo].asset);
			m_handler->getWorld()->addObjectAndInit(m_assets[AT_Logo].object);
		}
		m_handler->getWorld()->addToRenderList(m_assets[AT_Logo].object);
		emit renderUpdate();
	}

	void SceneControl::renderPlain()
	{
		constexpr float PLANE_LENGTH = 256.f;
		constexpr float PLANE_WIDTH = 256.f;
		constexpr float HALF_PLANE_LENGTH = PLANE_LENGTH / 2.f;
		constexpr float HALF_PLANE_WIDTH = PLANE_WIDTH / 2.f;
		constexpr float PLANE_HEIGHT = 2.f;
		if (m_assets[AT_Plane].asset.isEmpty())
		{
			// 创建一个平面
			GMPlainDescription desc = {
				-HALF_PLANE_LENGTH,
				0,
				-HALF_PLANE_WIDTH,
				PLANE_LENGTH,
				PLANE_WIDTH,
				50,
				50,
				{ 1, 1, 1 }
			};

			utCreatePlain(desc, m_assets[AT_Plane].asset);
			m_assets[AT_Plane].object = new GMGameObject(m_assets[AT_Plane].asset);
			m_handler->getWorld()->addObjectAndInit(m_assets[AT_Plane].object);

			// 设置一个物理形状
			GMRigidPhysicsObject* rigidPlain = new gm::GMRigidPhysicsObject();
			rigidPlain->setMass(.0f);
			m_assets[AT_Plane].object->setPhysicsObject(rigidPlain);
			GMPhysicsShapeHelper::createCubeShape(GMVec3(HALF_PLANE_LENGTH, PLANE_HEIGHT, HALF_PLANE_WIDTH), m_assets[AT_Plane].shape);
			rigidPlain->setShape(m_assets[AT_Plane].shape);

			// 添加到世界
			m_handler->getPhysicsWorld()->addRigidObject(rigidPlain);
		}
		m_handler->getWorld()->addToRenderList(m_assets[AT_Plane].object);
		emit renderUpdate();
	}

	void SceneControl::resetModel(SceneModel* model)
	{
		// 负责重置Model，更新渲染，更新资源
		m_model = model;
		
		// 重新生成场景相关的资源:
		// 重置摄像机
		m_sceneViewCamera.setPerspective(Radian(75.f), .75f, .1f, 2000); //TODO aspect需要计算，near和far需要从全局拿
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

	Asset SceneControl::hitTest(int x, int y)
	{
		const GMCamera& camera = currentCamera();
		GMVec3 rayFrom = camera.getLookAt().position;
		GMVec3 rayTo = camera.getRayToWorld(m_handler->getContext()->getWindow()->getRenderRect(), x, y);
		GMPhysicsRayTestResult rayTestResult = m_handler->getPhysicsWorld()->rayTest(rayFrom, rayTo);
		return findAsset(rayTestResult.hitObject);
	}

	Asset SceneControl::findAsset(GMPhysicsObject* phyObj)
	{
		if (phyObj)
		{
			for (int i = AT_Begin; i < AT_End; ++i)
			{
				if (m_assets[i].object && m_assets[i].object->getPhysicsObject() == phyObj)
					return m_assets[i];
			}
		}
		return Asset();
	}

}
