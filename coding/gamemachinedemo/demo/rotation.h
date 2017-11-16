#ifndef __DEMO_ANIMATION_H__
#define __DEMO_ANIMATION_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"
#include "normalmap.h"

GM_PRIVATE_OBJECT(Demo_Rotation)
{
	gm::linear_math::Quaternion rotation;
	gm::GMfloat angle = 0.f;
	bool rotate = true;
	gm::GMGameObject* gameObject = nullptr;
};

class Demo_Rotation : public Demo_NormalMap
{
	DECLARE_PRIVATE(Demo_Rotation)

	typedef Demo_NormalMap Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif