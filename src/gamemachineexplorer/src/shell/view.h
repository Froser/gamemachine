#ifndef __SHELL_VIEW_H__
#define __SHELL_VIEW_H__
#include <gmecommon.h>
#include "gamemachinewidget.h"

namespace shell
{
	class View : public GameMachineWidget
	{
		Q_OBJECT

		using GameMachineWidget::GameMachineWidget;

	protected:
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent(QMouseEvent* e) override;
		virtual void mouseMoveEvent(QMouseEvent* e) override;

	signals:
		void mousePress(View*, QMouseEvent*);
		void mouseRelease(View*, QMouseEvent*);
		void mouseMove(View*, QMouseEvent*);
	};
}

#endif