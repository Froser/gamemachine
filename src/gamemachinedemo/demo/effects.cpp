#include "stdafx.h"
#include "effects.h"

void Demo_Effects::onActivate()
{
	Base::onActivate();
	// 使用预设特效非常简单，只需要设置此状态即可
	// 如果是自定义特效，需要自行修改着色器
	GMSetRenderState(EFFECTS, gm::GMEffects::Blur);
}