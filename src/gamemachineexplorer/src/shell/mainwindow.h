#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__
#include <gmecommon.h>
#include <QMainWindow>

namespace shell
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		using QMainWindow::QMainWindow;
	};
}

#endif