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
		m_gmwidget = GameMachineWidget::createGameMachineWidget(desc, new core::Handler(), this);

		//mainLayout->addWidget(m_gmwidget);
		m_gmwidget->setGeometry(0, 0, 800, 600);
	}

	void MainWindow::setupUi()
	{
		resize(800, 600);

		m_centralwidget = new QWidget(this);
		m_centralwidget->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(m_centralwidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(m_centralwidget);
		mainLayout->setContentsMargins(0, 0, 0, 0);
	}

}