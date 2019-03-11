#include "stdafx.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include "src/core/handler.h"

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
		m_gmwidget->setGameMachine(desc, new core::Handler());
	}

	void MainWindow::setupUi()
	{
		resize(800, 600);

		m_centralwidget = new QWidget(this);
		m_centralwidget->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(m_centralwidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(m_centralwidget);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		//mainLayout->addWidget(m_gmwidget);
		m_gmwidget = new GameMachineWidget(m_centralwidget);
		m_gmwidget->setGeometry(0, 0, 800, 600);
	}

}