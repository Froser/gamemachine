#include "stdafx.h"
#include "mainwindow_actions.h"
#include <core/scene.h>
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

}
