#include "stdafx.h"
#include "scene_control.h"
#include <gmlight.h>
#include "handler.h"
#include "scene_model.h"
#include "render_tree.h"
#include "nodes/common_nodes.h"

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
		setCamera(defaultCamera());
	}

	void SceneControl::onSceneModelCreated(SceneModel* model)
	{
		resetModel(model);
		clearRenderList();
		setCurrentRenderTree(getRenderTree_Scene());
		render();
	}

	void SceneControl::onSceneModelDestroyed(SceneModel* model)
	{
		m_model = nullptr;
		if (m_sceneTree)
			m_sceneTree->reset();
		if (m_splashTree)
		{
			m_splashTree->reset();
			renderSplash();
		}
	}

	void SceneControl::onWidgetMousePress(shell::View* v, QMouseEvent* e)
	{
		if (m_currentRenderTree)
		{
			RenderNode* hitTestResult = m_currentRenderTree->hitTest(e->pos().x(), e->pos().y());
			m_currentRenderTree->onMousePress(mouseDetails(e), hitTestResult);
		}
		m_mouseDownPos = e->pos();
		m_mouseDown = true;
	}

	void SceneControl::onWidgetMouseRelease(shell::View* v, QMouseEvent* e)
	{
		if (m_currentRenderTree)
			m_currentRenderTree->onMouseRelease(mouseDetails(e));
		m_mouseDownPos = e->pos();
		m_mouseDown = false;
	}

	void SceneControl::onWidgetMouseMove(shell::View*, QMouseEvent* e)
	{
		if (m_currentRenderTree)
			m_currentRenderTree->onMouseMove(mouseDetails(e));
		m_mouseDownPos = e->pos();
	}

	Handler* SceneControl::getHandler()
	{
		return m_handler;
	}

	const GMCamera& SceneControl::viewCamera()
	{
		return m_sceneViewCamera;
	}

	void SceneControl::setViewCamera(const GMCamera& camera)
	{
		m_sceneViewCamera = camera;
	}

	void SceneControl::setCamera(const GMCamera& camera)
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

	void SceneControl::update()
	{
		emit renderUpdate();
	}

	void SceneControl::render()
	{
		if (m_currentRenderTree)
			m_currentRenderTree->render(true);
		emit renderUpdate();
	}

	void SceneControl::init()
	{
		clearRenderList();
		setDefaultColor(GMVec4(.117f, .117f, .117f, 1));
		renderSplash();
	}

	void SceneControl::resetModel(SceneModel* model)
	{
		// 负责重置Model，更新渲染，更新资源
		m_model = model;
		
		// 重新生成场景相关的资源:
		// 重置摄像机
		IWindow* window = m_handler->getContext()->getWindow();
		GMRect rc = window->getRenderRect();
		float aspect = rc.width / rc.height;
		m_sceneViewCamera.setPerspective(Radian(75.f), aspect, .1f, 2000); //TODO ，near和far需要从全局拿
		GlobalProperties& props = model->getProperties();
		GMCameraLookAt lookAt = GMCameraLookAt::makeLookAt(
			GMVec3(props.viewCamera.posX, props.viewCamera.posY, props.viewCamera.posZ),
			GMVec3(props.viewCamera.lookAtX, props.viewCamera.lookAtY, props.viewCamera.lookAtZ));
		m_sceneViewCamera.lookAt(lookAt);
		setCamera(m_sceneViewCamera);
	}

	RenderNode* SceneControl::hitTest(int x, int y)
	{
		if (m_currentRenderTree)
		{
			return m_currentRenderTree->hitTest(x, y);
		}

		return nullptr;
	}

	void SceneControl::setCurrentRenderTree(RenderTree* tree)
	{
		m_currentRenderTree = tree;
	}

	RenderMouseDetails SceneControl::mouseDetails(const QMouseEvent* e)
	{
		RenderMouseDetails details;
		details.position[0] = e->pos().x();
		details.position[1] = e->pos().y();
		details.lastPosition[0] = m_mouseDownPos.x();
		details.lastPosition[1] = m_mouseDownPos.y();
		details.mouseDown = m_mouseDown;
		return details;
	}

	RenderTree* SceneControl::getRenderTree_Splash()
	{
		if (!m_splashTree)
		{
			m_splashTree = new RenderTree(this);
			m_splashTree->appendNode(new SplashNode());
		}
		return m_splashTree;
	}

	RenderTree* SceneControl::getRenderTree_Scene()
	{
		if (!m_sceneTree)
		{
			m_sceneTree = new RenderTree(this);
			m_sceneTree->appendNode(new PlaneNode());
		}
		return m_sceneTree;
	}

	void SceneControl::renderSplash()
	{
		setCamera(defaultCamera());
		setCurrentRenderTree(getRenderTree_Splash());
		render();
	}

}
