#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__
#include <gmecommon.h>
#include <QMainWindow>
#include <QtWidgets>
#include <gamemachine.h>
#include "gamemachinewidget.h"

namespace shell
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow(QWidget* parent = nullptr);

	public:
		void initGameMachine(const GMGameMachineDesc&);

	private:
		void setupUi();

	private:
		QWidget* m_centralwidget = nullptr;
		GameMachineWidget* m_gmwidget = nullptr;
		core::HandlerControl* m_gmcontrol = nullptr;
	};
}

#endif