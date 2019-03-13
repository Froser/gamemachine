#include "stdafx.h"
#include "mainwindow_actions.h"
#include <core/scene.h>

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
}
