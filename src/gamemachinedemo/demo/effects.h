#ifndef __DEMO_EFFECTS_H__
#define __DEMO_EFFECTS_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"

class Demo_Effects : public Demo_Texture
{
	typedef Demo_Texture Base;

public:
	using Base::Base;

public:
	virtual void onActivate() override;
};

#endif