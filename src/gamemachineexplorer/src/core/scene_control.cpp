#include "stdafx.h"
#include "scene_control.h"
#include <gmlight.h>
#include "handler.h"
#include "scene_model.h"
#include "render_tree.h"

namespace
{
	const GMVec3 s_x(1, 0, 0);
	const GMVec3 s_up(0, 1, 0);
	const GMVec3 s_z(0, 0, 1);

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
	}

	void SceneControl::onWidgetMousePress(shell::View* v, QMouseEvent* e)
	{
		if (m_currentRenderTree)
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
			float dz = e->pos().y() - m_mouseDownPos.y();
			
			// 如果选中了地面，移动镜头
			if (m_selectedAssets.contains(m_assets[AT_Plane]))
			{
				// 镜头位置移动
				GMCameraLookAt lookAt = m_sceneViewCamera.getLookAt();
				
				// 镜头位置在相机空间的坐标
				GMVec4 position_Camera = GMVec4(dx, 0, dz, 1);

				// 创建相机坐标系
				GMVec3 cameraZ_World = Normalize(GMVec3(lookAt.lookDirection.getX(), 0, lookAt.lookDirection.getZ()));

				// 求出相机x坐标与世界坐标余弦
				GMVec3 cameraX_World = Normalize(Cross(s_up, cameraZ_World));
				float cos_X = Dot(cameraX_World, s_x);
				float sin_X = Sqrt(1 - cos_X * cos_X);
				GMVec3 posDelta = GMVec3(dx * cos_X + dz * sin_X, 0, dx * sin_X + dz * cos_X);
				lookAt.position = lookAt.position + posDelta;

				m_sceneViewCamera.lookAt(lookAt);
				setViewCamera(m_sceneViewCamera);
				m_mouseDownPos = e->pos();
				emit renderUpdate();
			}
		}
	}

	Handler* SceneControl::getHandler()
	{
		return m_handler;
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

	RenderNode* SceneControl::hitTest(int x, int y)
	{
		if (m_currentRenderTree)
		{
			return m_currentRenderTree->hitTest(x, y);
		}
	}
}
