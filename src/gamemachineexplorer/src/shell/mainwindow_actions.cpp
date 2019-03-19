#include "stdafx.h"
#include "mainwindow_actions.h"
#include <core/scene.h>
#include <core/scene_model.h>
#include <QApplication>

namespace shell
{
	void MainWindowActions::setScene(core::Scene* scene)
	{
		m_scene = scene;
	}

	void MainWindowActions::onNewFile()
	{
		if (m_scene)
		{
			m_scene->newFile();
		}
	}

	void MainWindowActions::onCloseFile()
	{
		if (m_scene)
		{
			m_scene->closeFile();
		}
	}

	void MainWindowActions::onQuit()
	{
		QApplication::instance()->quit();
	}

	void MainWindowActions::onOrtho()
	{
		if (m_scene)
		{
			core::SceneModel* model = m_scene->model();
			if (model)
			{
				model->getProperties().viewCamera.ortho = 1;
				model->notifyPropertyChanged(core::Property_ViewCamera);
			}
		}
	}

	void MainWindowActions::onPerspective()
	{
		if (m_scene)
		{
			core::SceneModel* model = m_scene->model();
			if (model)
			{
				model->getProperties().viewCamera.ortho = 0;
				model->notifyPropertyChanged(core::Property_ViewCamera);
			}
		}
	}

}
