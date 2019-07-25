#ifndef __GMWINDOW_P_H__
#define __GMWINDOW_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMWindow)
{
	GMOwnedPtr<IInput> input;
	GMOwnedPtr<IGameHandler> handler;
	GMOwnedPtr<IGraphicEngine> engine;
	GMOwnedPtr<IRenderContext> context;

	GMWindowHandle handle = 0;
	bool ownedHandle = true; //!< 是否控制原生窗口句柄生命周期

	Vector<GMWidget*> widgets;
	GMWindowStates windowStates;
	GMCursorType cursor = GMCursorType::Arrow;
};

END_NS
#endif