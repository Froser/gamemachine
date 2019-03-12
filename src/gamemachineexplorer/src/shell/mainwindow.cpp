#include "stdafx.h"
#include "mainwindow.h"
#include <gamemachinewidget.h>
#include "src/core/handler.h"
#include "src/core/handler_control.h"

namespace shell
{
	MainWindow::MainWindow(QWidget* parent /*= nullptr*/)
		: QMainWindow(parent)
	{
		setupUi();
	}

	void MainWindow::initGameMachine(const GMGameMachineDesc& desc)
	{
		Q_ASSERT(m_gmwidget);
		core::Handler* handler = new core::Handler();
		m_gmwidget->setGameMachine(desc, handler);
		
		m_gmcontrol = new core::HandlerControl(handler, this);
		m_gmcontrol->clearRenderList();
		m_gmcontrol->renderLogo();
		m_gmcontrol->setDefaultColor(GMVec4(.117f, .117f, .117f, 1));
	}

	void MainWindow::setupUi()
	{
		resize(800, 600);

		m_centralwidget = new QWidget(this);
		m_centralwidget->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(m_centralwidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(m_centralwidget);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		m_gmwidget = new GameMachineWidget(m_centralwidget);
		m_gmwidget->setGeometry(0, 0, 800, 600);
	}

}