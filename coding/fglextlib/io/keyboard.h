#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include "common.h"
BEGIN_NS

struct Keyboard
{
	static bool isKeyDown(int vk);
	static bool isKeyUp(int vk);
};

END_NS
#endif