#include "stdafx.h"
#include "lua.h"
#include <linearmath.h>
#include <gmwidget.h>
#include <gmcontrolbutton.h>

void Demo_Lua::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	// 创建对象
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	gm::GMRect txtCorner = { 0, 0, 6, 8 };
	gm::GMRect thumbCorner = { 0, 0, 7, 15 };
	d->textCode = nullptr;
	widget->addControl(d->textCode = gm::GMControlTextArea::createControl(
		widget,
		L"",
		10,
		top,
		600,
		500,
		false,
		true,
		txtCorner,
		thumbCorner
	));
	d->textCode->setPadding(5, 10);
	d->textCode->setScrollBar(true);

	gm::GMControlButton* btnRun = nullptr;
	widget->addControl(btnRun = gm::GMControlButton::createControl(
		widget,
		L"运行",
		10,
		top += 520,
		250,
		30,
		false
	));
	connect(*btnRun, GM_SIGNAL(gm::GMControlButton, click), [=](auto, auto) {
		runScript();
	});

	widget->addControl(d->lbState = gm::GMControlLabel::createControl(
		widget,
		L"",
		GMVec4(1, 1, 1, 1),
		10,
		top += 40,
		250,
		30,
		false
	));

	widget->setSize(800, top);
}

void Demo_Lua::event(gm::GameMachineHandlerEvent evt)
{
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

gm::GMWidget* Demo_Lua::createDefaultWidget()
{
	D_BASE(d, Base);
	gm::GMFontHandle stxingka = d->engine->getGlyphManager()->addFontByFileName("STXINGKA.TTF");
	d->mainWidget = gm_makeOwnedPtr<gm::GMWidget>(getDemonstrationWorld()->getManager());
	getDemonstrationWorld()->getManager()->registerWidget(d->mainWidget.get());
	getDemonstrationWorld()->getUIConfiguration()->initWidget(d->mainWidget.get());

	d->mainWidget->setTitle(L"选项菜单");
	d->mainWidget->setTitleVisible(true);

	if (stxingka != gm::GMInvalidFontHandle)
	{
		gm::GMStyle style = d->mainWidget->getTitleStyle();
		style.setFont(stxingka);
		d->mainWidget->setTitleStyle(style);
	}

	d->mainWidget->setKeyboardInput(true);
	d->mainWidget->setVisible(false);

	gm::GMRect corner = { 0,0,75,42 };
	d->mainWidget->addBorder(corner);
	d->mainWidget->setPosition(10, 60);
	d->mainWidget->setSize(300, 500);
	getDemoWorldReference()->getContext()->getWindow()->addWidget(d->mainWidget.get());

	gm::GMint32 top = 10;

	if (!getDescription().isEmpty())
	{
		d->mainWidget->addControl(gm::GMControlLabel::createControl(
			d->mainWidget.get(),
			getDescription(),
			getLabelFontColor(),
			10,
			top,
			250,
			30,
			false
		));
		top += 40;
	}

	gm::GMControlButton* button = nullptr;
	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"返回主菜单",
		10,
		top,
		250,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		backToEntrance();
	});

	d->nextControlTop = top + 40;
	d->mainWidget->setSize(d->mainWidget->getSize().width, d->nextControlTop);
	return d->mainWidget.get();
}

void Demo_Lua::runScript()
{
	D(d);
	gm::GMLuaResult lr = d->lua.runString(d->textCode->getText());
	if (lr.state == gm::GMLuaStates::SyntaxError)
		d->lbState->setText(L"Lua脚本出现语法错误: " + lr.message);
	else
		d->lbState->setText(L"Lua执行成功。");
}