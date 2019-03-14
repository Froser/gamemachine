#include "stdafx.h"
#include "scene.h"
#include "scene_model.h"
#include "scene_control.h"
#include <shell/view.h>

namespace core
{
	void Scene::newFile()
	{
		closeFile();
		m_model = new SceneModel(this);
		emit fileCreated(m_model);
	}

	void Scene::closeFile()
	{
		if (m_model)
		{
			emit fileClosed(m_model);
			m_model->deleteLater();
			m_model = nullptr;
		}
	}

	SceneControl* Scene::control()
	{
		return m_control;
	}

	shell::View* Scene::widget()
	{
		return m_widget;
	}

	SceneModel* Scene::model()
	{
		return m_model;
	}

	void Scene::setControl(SceneControl* control)
	{
		Q_ASSERT(!m_control);
		m_control = control;
		connect(m_control, &SceneControl::renderUpdate, this, &Scene::onRenderUpdate);
		connect(this, &Scene::fileCreated, m_control, &SceneControl::onSceneModelCreated, Qt::UniqueConnection);
		connect(this, &Scene::fileClosed, m_control, &SceneControl::onSceneModelDestroyed, Qt::UniqueConnection);
		connectWidgetAndControl();
	}

	void Scene::setWidget(shell::View* widget)
	{
		Q_ASSERT(!m_widget);
		m_widget = widget;
		connectWidgetAndControl();
	}

	void Scene::setModel(SceneModel* model)
	{
		m_model = model;
	}

	void Scene::onRenderUpdate()
	{
		if (widget())
			widget()->repaint();
	}

	void Scene::connectWidgetAndControl()
	{
		if (m_control && m_widget)
		{
			connect(m_widget, &shell::View::mousePress, m_control, &SceneControl::onWidgetMousePress, Qt::UniqueConnection);
			connect(m_widget, &shell::View::mouseRelease, m_control, &SceneControl::onWidgetMouseRelease, Qt::UniqueConnection);
			connect(m_widget, &shell::View::mouseMove, m_control, &SceneControl::onWidgetMouseMove, Qt::UniqueConnection);
		}
	}
}