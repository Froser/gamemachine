#ifndef __GM_LUA_IINPUT_META_H__
#define __GM_LUA_IINPUT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gminput.h>
BEGIN_NS

GM_PRIVATE_OBJECT(IInputProxy)
{
	GM_LUA_PROXY(IInput);
	GM_LUA_PROXY_FUNC(getKeyboardState);
	GM_LUA_PROXY_FUNC(getJoystickState);
	GM_LUA_PROXY_FUNC(getMouseState);
};

class IInputProxy : public GMObject
{
	GM_LUA_PROXY_OBJECT(IInputProxy, IInput)

protected:
	virtual bool registerMeta() override;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(IKeyboardStateProxy)
{
	GM_LUA_PROXY(IKeyboardState);
	GM_LUA_PROXY_FUNC(keydown);
	GM_LUA_PROXY_FUNC(keyTriggered);
};

class IKeyboardStateProxy : public GMObject
{
	GM_LUA_PROXY_OBJECT(IKeyboardStateProxy, IKeyboardState)

protected:
	virtual bool registerMeta() override;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(IMouseStateProxy)
{
	GM_LUA_PROXY(IMouseState);
	GM_LUA_PROXY_FUNC(mouseState);
	GM_LUA_PROXY_FUNC(setDetectingMode);
};

class IMouseStateProxy : public GMObject
{
	GM_LUA_PROXY_OBJECT(IMouseStateProxy, IMouseState)

protected:
	virtual bool registerMeta() override;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(IJoystickStateProxy)
{
	GM_LUA_PROXY(IJoystickState);
	GM_LUA_PROXY_FUNC(joystickVibrate);
	GM_LUA_PROXY_FUNC(joystickState);
};

class IJoystickStateProxy : public GMObject
{
	GM_LUA_PROXY_OBJECT(IJoystickStateProxy, IJoystickState)

protected:
	virtual bool registerMeta() override;
};

END_NS
#endif