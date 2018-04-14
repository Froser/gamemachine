#include "stdafx.h"
#include "effects.h"

void Demo_Effects::onActivate()
{
	D_BASE(d, Base::Base);
	Base::onActivate();
	// 使用预设特效非常简单，只需要设置此状态即可
	// 如果是自定义特效，需要自行修改着色器
	d->renderConfig.set(gm::GMRenderConfigs::FilterMode, gm::GMFilterMode::Grayscale);
}