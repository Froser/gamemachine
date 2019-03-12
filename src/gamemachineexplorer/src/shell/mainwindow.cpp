#include "stdafx.h"
#include "mainwindow.h"
#include <gamemachinewidget.h>
#include <core/scene.h>
#include <core/handler.h>
#include <core/handler_control.h>
#include <core/scene_model.h>

namespace shell
{
	MainWindow::MainWindow(QWidget* parent /*= nullptr*/)
		: QMainWindow(parent)
	{
		setupUi();
	}

	void MainWindow::initGameMachine(const GMGameMachineDesc& desc)
	{
		GameMachineWidget* gmwidget = m_scene->widget();
		if (gmwidget)
		{
			core::Handler* handler = new core::Handler();
			gmwidget->setGameMachine(desc, size().width(), size().height(), handler);

			core::HandlerControl* control = new core::HandlerControl(handler, m_scene);
			control->clearRenderList();
			control->renderLogo();
			control->setDefaultColor(GMVec4(.117f, .117f, .117f, 1));
			m_scene->setControl(control);
			m_scene->setModel(new core::SceneModel(m_scene));
		}
	}

	void MainWindow::setupUi()
	{
		resize(1024, 768);

		// Create main layout
		m_centralwidget = new QWidget(this);
		m_centralwidget->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(m_centralwidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(m_centralwidget);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		GameMachineWidget* gmwidget = new GameMachineWidget(m_centralwidget);
		m_scene = new core::Scene(this);
		m_scene->setWidget(gmwidget);
		mainLayout->addWidget(gmwidget);

		// Create menus and toolbar
		createActions();
		createMenus();
		createToolbar();
	}

	void MainWindow::createActions()
	{
		if (!m_newFileAct)
		{
			m_newFileAct = new QAction(tr("New"), this);
		}

		if (!m_openFileAct)
		{
			m_openFileAct = new QAction(tr("Open"), this);
		}

		if (!m_addModelAct)
		{
			m_addModelAct = new QAction(tr("Add Model"), this);
		}

		if (!m_addLightAct)
		{
			m_addLightAct = new QAction(tr("Add Light"), this);
		}
	}

	void MainWindow::createMenus()
	{
		auto fileMenu = menuBar()->addMenu(tr("&File"));
		fileMenu->addAction(m_newFileAct);
		fileMenu->addAction(m_openFileAct);
	}

	void MainWindow::createToolbar()
	{
		auto basicTb = addToolBar(tr("File"));
		basicTb->addAction(m_newFileAct);
		basicTb->addAction(m_openFileAct);

		auto itemTb = addToolBar(tr("Items"));
		itemTb->addAction(m_addModelAct);
		itemTb->addAction(m_addLightAct);
	}

}