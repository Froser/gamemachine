#include "stdafx.h"
#include "scene_control.h"
#include <gmlight.h>
#include "handler.h"
#include "scene_model.h"
#include "render_tree.h"
#include "nodes/common_nodes.h"

namespace core
{
	SceneControl::SceneControl(Handler* handler, QObject* parent /*= nullptr*/)
		: QObject(parent)
		, m_handler(handler)
	{
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

	Handler* SceneControl::handler()
	{
		return m_handler;
	}

	SceneModel* SceneControl::model()
	{
		return m_model;
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

	void SceneControl::setClearColor(const GMVec4& color)
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->getDefaultFramebuffers()->setClearColor(ValuePointer(color));
	}

	ILight* SceneControl::defaultLight()
	{
		auto engine = m_handler->getContext()->getEngine();
		ILight* light = nullptr;
		if (m_defaultLight.defaultLightIndex == -1)
		{
			GM.getFactory()->createLight(GMLightType::PointLight, &light);
			m_defaultLight.defaultLightIndex = engine->addLight(light);
		}
		else
		{
			light = engine->getLight(m_defaultLight.defaultLightIndex);
		}
		return light;
	}

	void SceneControl::updateLight()
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->update(GMUpdateDataType::LightChanged);
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
		setClearColor(GMVec4(.117f, .117f, .117f, 1));
		renderSplash();
	}

	void SceneControl::resetModel(SceneModel* model)
	{
		// 负责重置Model
		m_model = model;
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
		if (m_currentRenderTree != tree)
		{
			m_currentRenderTree = tree;
			if (m_currentRenderTree)
				m_currentRenderTree->onRenderTreeSet();
		}
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
			m_splashTree = new SplashRenderTree(this);
			m_splashTree->appendNode(new SplashNode());
		}
		return m_splashTree;
	}

	RenderTree* SceneControl::getRenderTree_Scene()
	{
		if (!m_sceneTree)
		{
			m_sceneTree = new SceneRenderTree(this);
			m_sceneTree->appendNode(new PlaneNode());
		}
		return m_sceneTree;
	}

	void SceneControl::renderSplash()
	{
		setCurrentRenderTree(getRenderTree_Splash());
		render();
	}
}
