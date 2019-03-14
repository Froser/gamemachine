#ifndef __SHELL_MAINWINDOW_ACTIONS_H__
#define __SHELL_MAINWINDOW_ACTIONS_H__
#include <gmecommon.h>
#include <QtCore>

namespace shell
{
	class MainWindowActions : public QObject
	{
		Q_OBJECT
		using QObject::QObject;

	public:
		void setScene(core::Scene* scene);

	public slots:
		void onNewFile();
		void onCloseFile();
		void onQuit();

	private:
		core::Scene* m_scene = nullptr;
	};
}

#endif