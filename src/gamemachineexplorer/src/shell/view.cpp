#include "stdafx.h"
#include "view.h"

namespace shell
{

	void View::mousePressEvent(QMouseEvent* e)
	{
		emit mousePress(this, e);
	}

	void View::mouseReleaseEvent(QMouseEvent* e)
	{
		emit mouseRelease(this, e);
	}

	void View::mouseMoveEvent(QMouseEvent* e)
	{
		emit mouseMove(this, e);
	}

}