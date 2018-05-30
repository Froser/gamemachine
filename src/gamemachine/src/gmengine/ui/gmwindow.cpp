#include "stdafx.h"
#include "gmwindow.h"
#include "foundation/gmmessage.h"
#include "foundation/gamemachine.h"

bool GMWindow::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}

bool GMWindow::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMWindow::handleSystemEvent(GMSystemEvent* event, REF GMLResult& result)
{
	if (!event)
		return false;

	switch (event->getType())
	{
	case GMSystemEventType::Destroy:
		destroyWindow();
		break;
	case GMSystemEventType::WindowSizeChanged:
		GM.postMessage({ GameMachineMessageType::WindowSizeChanged });
		break;
	case GMSystemEventType::MouseWheel:
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
		getInputMananger()->msgProc(event);
		break;
	}

	return GM.dispatchEventToCanvases(event);
}
