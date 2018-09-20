#include "stdafx.h"
#include "check.h"
#include "../../gamemachine.h"
#include <locale.h>
#include <X11/Xlib.h>
#include "window/gmxrendercontext.h"

namespace
{
	GMEndiannessMode getMachineEndianness()
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			return GMEndiannessMode::LittleEndian;
		return GMEndiannessMode::BigEndian;
	}

	IWindow* findWindow(const Set<GMOwnedPtr<IWindow>>& windows, Window window)
	{
		for (const auto& w : windows)
		{
			if (w->getWindowHandle() == window)
				return w.get();
		}
		return nullptr;
	}

	GMMouseButton translateButton(GMuint state)
	{
		GMMouseButton button = GMMouseButton_None;
		if (state & Button1Mask)
			button |= GMMouseButton_Left;
		if (state & Button2Mask)
			button |= GMMouseButton_Middle;
		if (state & Button3Mask)
			button |= GMMouseButton_Right;
		return button;
	}

	GMModifier translateModifier(GMuint state)
	{
		GMModifier modifier = GMModifier_None;
		if (state & ControlMask)
			modifier |= GMModifier_Ctrl;
		if (state & ShiftMask)
			modifier |= GMModifier_Shift;
		return modifier;
	}

	GMKey translateKey(XKeyEvent* xkey)
	{
		XComposeStatus composeStatus;
		char asciiCode[32];
		KeySym keySym;
		GMint len = 0;
		len = XLookupString(xkey, asciiCode, sizeof(asciiCode), &keySym, &composeStatus);
		if (len > 0)
		{
			return GM_ASCIIToKey(asciiCode[0]);
		}
		else
		{
			switch (keySym)
			{
				case XK_F1:
					return GMKey_F1;
				case XK_F2:
					return GMKey_F2;
				case XK_F3:
					return GMKey_F3;
				case XK_F4:
					return GMKey_F4;
				case XK_F5:
					return GMKey_F5;
				case XK_F6:
					return GMKey_F6;
				case XK_F7:
					return GMKey_F7;
				case XK_F8:
					return GMKey_F8;
				case XK_F9:
					return GMKey_F9;
				case XK_F10:
					return GMKey_F10;
				case XK_F11:
					return GMKey_F11;
				case XK_F12:
					return GMKey_F12;
				case XK_KP_Left:
				case XK_Left:
					return GMKey_Left;
				case XK_KP_Right:
				case XK_Right:
					return GMKey_Right;
				case XK_KP_Up:
				case XK_Up:
					return GMKey_Up;
				case XK_KP_Down:
				case XK_Down:
					return GMKey_Down;
				case XK_KP_Prior:
				case XK_Prior:
					return GMKey_Prior;
				case XK_KP_Next:
				case XK_Next:
					return GMKey_Next;
				case XK_KP_Home:
				case XK_Home:
					return GMKey_Home;
				case XK_KP_End:
				case XK_End:
					return GMKey_End;
				case XK_KP_Insert:
				case XK_Insert:
					return GMKey_Insert;
				case XK_Num_Lock:
					return GMKey_Numlock;
				case XK_KP_Delete:
					return GMKey_Delete;
				case XK_Shift_L:
				case XK_Shift_R:
					return GMKey_Shift;
				case XK_Control_L:
				case XK_Control_R:
					return GMKey_Control;
				//TODO Key Begin, Key Alt
			}
		}
	}
}

void GameMachine::runEventLoop()
{
	D(d);
	const GMXRenderContext* context = nullptr;
	if (d->windows.size() > 0)
	{
		IWindow* window = d->windows.begin()->get();
		context = gm_cast<const GMXRenderContext*>(window->getContext());
	}

	XEvent e;
	while (1)
	{
		if (context)
		{
			Display* display = context->getDisplay();
			while (XPending(display))
			{
				XNextEvent(display, &e);
				IWindow* window = findWindow(d->windows, e.xany.window);
				if (window)
				{
					GMXEventContext c = { &e, window };
					window->getProcHandler()(window->getWindowHandle(), 0, 0, reinterpret_cast<GMLParam>(&c));
				}
			}
		}

		if (!renderFrame())
			break;
	}
	terminate();
}

void GameMachine::translateSystemEvent(GMuint uMsg, GMWParam wParam, GMLParam lParam, OUT GMSystemEvent** event)
{
	GM_ASSERT(event);
	GMXEventContext* c = reinterpret_cast<GMXEventContext*>(lParam);
	GMWindow* window = gm_cast<GMWindow*>(c->window);
	XEvent* xevent = c->event;

	GMSystemEvent* newSystemEvent = nullptr;
	GMKey key;

	switch (xevent->type)
	{
		case KeyPress:
		{
			key = translateKey(&xevent->xkey);
			newSystemEvent = new GMSystemKeyEvent(GMSystemEventType::KeyDown, key, translateModifier(xevent->xkey.state));
			break;
		}
		case KeyRelease:
		{
			key = translateKey(&xevent->xkey);
			newSystemEvent = new GMSystemKeyEvent(GMSystemEventType::KeyUp, key, translateModifier(xevent->xkey.state));
			break;
		}
		case ButtonPress:
		case ButtonRelease:
		{
			GMPoint mousePoint = { xevent->xbutton.x, xevent->xbutton.y };
			GMSystemEventType type = GMSystemEventType::Unknown;
			if (xevent->type == ButtonPress)
			{
				type = GMSystemEventType::MouseDown;
				// gm_debug(gm_dbg_wrap("Mouse down detected."));
			}
			else if (xevent->type == ButtonRelease)
			{
				type = GMSystemEventType::MouseUp;
				// gm_debug(gm_dbg_wrap("Mouse up detected."));
			}

			GMMouseButton triggeredButton = GMMouseButton_None;
			if (xevent->xbutton.button == Button1)
				triggeredButton = GMMouseButton_Left;
			else if (xevent->xbutton.button == Button2)
				triggeredButton = GMMouseButton_Middle;
			else if (xevent->xbutton.button == Button3)
				triggeredButton = GMMouseButton_Right;

			GM_ASSERT(type != GMSystemEventType::Unknown);
			newSystemEvent = new GMSystemMouseEvent(type, mousePoint, triggeredButton, translateButton(xevent->xbutton.state), translateModifier(xevent->xbutton.state));
			break;
		}
		case MotionNotify:
		{
			GMPoint mousePoint = { xevent->xmotion.x, xevent->xmotion.y };
			// gm_debug(gm_dbg_wrap("Mouse move detected. ({0}, {1})"), GMString(mousePoint.x), GMString(mousePoint.y));
			GMSystemEventType type = GMSystemEventType::MouseMove;
			GMMouseButton triggeredButton = GMMouseButton_None;
			if (xevent->xbutton.button == Button1)
				triggeredButton = GMMouseButton_Left;
			else if (xevent->xbutton.button == Button2)
				triggeredButton = GMMouseButton_Middle;
			else if (xevent->xbutton.button == Button3)
				triggeredButton = GMMouseButton_Right;
			newSystemEvent = new GMSystemMouseEvent(type, mousePoint, triggeredButton, translateButton(xevent->xbutton.state), translateModifier(xevent->xbutton.state));
			break;
		}
		default:
			newSystemEvent = new GMSystemEvent(); // Create an empty event
	}

	*event = newSystemEvent;
}

void GameMachine::initSystemInfo()
{
	D(d);
	static bool inited = false;
	if (!inited)
	{
		d->states.systemInfo.endiannessMode = getMachineEndianness();
		inited = true;

		// set locale
		char* result = setlocale(LC_ALL, "zh_CN.utf8");
		if (!result)
		{
			gm_error(gm_dbg_wrap("Set locale failed. Type 'sudo apt-get install language-pack-zh-hans' to install ch_ZN locale pack."));
		}
	}
}