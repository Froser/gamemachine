#include "stdafx.h"
#include "scene.h"
#include "scene_model.h"
#include "scene_control.h"
#include <gamemachinewidget.h>

namespace core
{
	void Scene::newFile()
	{
		if (m_model)
			m_model->deleteLater();

		m_model = new SceneModel(this);
		connect(this, &Scene::fileCreated, m_control, &SceneControl::onSceneModelCreated, Qt::UniqueConnection);
		emit fileCreated(m_model);
	}

	SceneControl* Scene::control()
	{
		return m_control;
	}

	GameMachineWidget* Scene::widget()
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
	}

	void Scene::setWidget(GameMachineWidget* widget)
	{
		Q_ASSERT(!m_widget);
		m_widget = widget;
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
}