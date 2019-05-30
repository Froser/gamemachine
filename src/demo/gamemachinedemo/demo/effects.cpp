#include "stdafx.h"
#include "effects.h"
#include <gmcontrols.h>
#include <gmwidget.h>

#define DECL_BUTTON(button, top, effect, offset, text) \
widget->addControl(button = gm::GMControlButton::createControl( \
widget,				\
text,				\
10,					\
top+=(offset),		\
250,				\
30,					\
false				\
));					\
connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) { setEffect(effect); });

void Demo_Effects::init()
{
	D_BASE(d, Base);
	Base::init();

	auto top = getClientAreaTop();
	gm::GMControlButton* button = nullptr;
	gm::GMWidget* widget = getWidget();

	DECL_BUTTON(button, top, gm::GMFilterMode::None, 0, L"无效果");
	DECL_BUTTON(button, top, gm::GMFilterMode::Inversion, 40, L"反色");
	DECL_BUTTON(button, top, gm::GMFilterMode::Sharpen, 40, L"锐化");
	DECL_BUTTON(button, top, gm::GMFilterMode::Blur, 40, L"模糊");
	DECL_BUTTON(button, top, gm::GMFilterMode::Grayscale, 40, L"灰度");
	DECL_BUTTON(button, top, gm::GMFilterMode::EdgeDetect, 40, L"边缘检测");

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Effects::onActivate()
{
	D(d);
	Base::onActivate();
	// 使用预设特效非常简单，只需要设置此状态即可
	// 如果是自定义特效，需要自行修改着色器
	setEffect(d->mode);
}

void Demo_Effects::setEffect(gm::GMFilterMode::Mode mode)
{
	D(d);
	D_BASE(db, Base);
	d->mode = mode;
	db->renderConfig.set(gm::GMRenderConfigs::FilterMode, mode);
}