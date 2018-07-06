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
	widget->addTextArea(
		L"",
		10,
		top,
		600,
		500,
		false,
		true,
		txtCorner,
		thumbCorner,
		&d->textCode
	);
	d->textCode->setPadding(5, 10);
	d->textCode->setScrollBar(true);

	gm::GMControlButton* btnRun = nullptr;
	widget->addButton(
		L"运行",
		10,
		top += 520,
		250,
		30,
		false,
		&btnRun
	);
	connect(*btnRun, GM_SIGNAL(gm::GMControlButton::click), [=](auto, auto) {
		runScript();
	});

	widget->addLabel(
		L"",
		GMVec4(1, 1, 1, 1),
		10,
		top += 40,
		250,
		30,
		false,
		&d->lbState
	);

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
	case gm::GameMachineHandlerEvent::Simulate:
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

	{
		gm::GMRect rc = { 0, 0, 136, 54 };
		d->mainWidget->addArea(gm::GMTextureArea::ButtonArea, rc);
	}
	{
		gm::GMRect rc = { 136, 0, 116, 54 };
		d->mainWidget->addArea(gm::GMTextureArea::ButtonFillArea, rc);
	}
	{
		gm::GMRect rc = { 8, 82, 238, 39 };
		d->mainWidget->addArea(gm::GMTextureArea::TextEditBorderArea, rc);
	}
	{
		gm::GMRect rc = { 0, 0, 280, 287 };
		d->mainWidget->addArea(gm::GMTextureArea::BorderArea, rc);
	}
	{
		gm::GMRect rc = { 196, 192, 22, 20 };
		d->mainWidget->addArea(gm::GMTextureArea::ScrollBarUp, rc);
	}
	{
		gm::GMRect rc = { 196, 223, 22, 20 };
		d->mainWidget->addArea(gm::GMTextureArea::ScrollBarDown, rc);
	}
	{
		gm::GMRect rc = { 220, 192, 18, 42 };
		d->mainWidget->addArea(gm::GMTextureArea::ScrollBarThumb, rc);
	}
	{
		gm::GMRect rc = { 196, 212, 22, 11 };
		d->mainWidget->addArea(gm::GMTextureArea::ScrollBarTrack, rc);
	}

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

	gm::GMint top = 10;

	if (!getDescription().isEmpty())
	{
		d->mainWidget->addLabel(
			getDescription(),
			getLabelFontColor(),
			10,
			top,
			250,
			30,
			false,
			nullptr
		);
		top += 40;
	}

	gm::GMControlButton* button = nullptr;
	d->mainWidget->addButton(
		L"返回主菜单",
		10,
		top,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		backToEntrance();
	});

	d->nextControlTop = top + 40;
	d->mainWidget->setSize(d->mainWidget->getSize().width, d->nextControlTop);
	return d->mainWidget.get();
}

void Demo_Lua::runScript()
{
	D(d);
	gm::GMBuffer buf;
	auto code = d->textCode->getText().toStdString();
	buf.buffer = const_cast<gm::GMbyte*>(reinterpret_cast<const gm::GMbyte*>(code.data()));
	buf.size = code.size() + 1;
	gm::GMLuaStatus state = d->lua.loadBuffer(buf);
	if (state == gm::GMLuaStatus::SyntaxError)
		d->lbState->setText(L"Lua脚本出现语法错误。");
	else
		d->lbState->setText(L"Lua执行成功。");
}