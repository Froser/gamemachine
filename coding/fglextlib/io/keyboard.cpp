#include "stdafx.h"
#include "keyboard.h"

#ifdef _WINDOWS
#	define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? true : false)
#	define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? false : true) 
#endif

bool Keyboard::isKeyDown(int vk)
{
#ifdef _WINDOWS
	return KEYDOWN(vk);
#else
	ASSERT(false);
	return false;
#endif
}

bool Keyboard::isKeyUp(int vk)
{
#ifdef _WINDOWS
	return KEYUP(vk);
#else
	ASSERT(false);
	return false;
#endif
}
