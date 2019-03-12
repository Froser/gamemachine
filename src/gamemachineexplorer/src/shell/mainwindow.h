#ifndef __SHELL_MAINWINDOW_H__
#define __SHELL_MAINWINDOW_H__
#include <gmecommon.h>
#include <QMainWindow>
#include <QtWidgets>
#include <gamemachine.h>

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
		void createActions();
		void createMenus();
		void createToolbar();
		void createToolbox();

	private:
		QWidget* m_centralwidget = nullptr;
		core::Scene* m_scene = nullptr;

	// Actions
	private:
		QAction* m_newFileAct = nullptr;
		QAction* m_openFileAct = nullptr;

		QAction* m_addModelAct = nullptr;
		QAction* m_addLightAct = nullptr;
	};
}

#endif