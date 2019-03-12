#include "stdafx.h"
#include "scene.h"

namespace core
{
	HandlerControl* Scene::control()
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

	void Scene::setControl(HandlerControl* control)
	{
		m_control = control;
	}

	void Scene::setWidget(GameMachineWidget* widget)
	{
		m_widget = widget;
	}

	void Scene::setModel(SceneModel* model)
	{
		m_model = model;
	}

}