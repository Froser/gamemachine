#include "stdafx.h"
#include "controls.h"
#include <linearmath.h>
#include <gmwidget.h>
#include <gmcontrols.h>
#include <gmcontroltextedit.h>

void Demo_Controls::init()
{
	D(d);
	Base::init();
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(d->parentDemonstrationWorld->getContext()));
	createDefaultWidget();
}

void Demo_Controls::event(gm::GameMachineHandlerEvent evt)
{
	D_BASE(db, Base);
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
		getDemoWorldReference()->updateGameWorld(GM.getRunningStates().lastFrameElpased);
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

gm::GMWidget* Demo_Controls::createDefaultWidget()
{
	D(d);
	gm::GMFontHandle stxingka = d->engine->getGlyphManager()->addFontByFileName("STXINGKA.TTF");
	d->mainWidget = gm_makeOwnedPtr<gm::GMWidget>(getDemonstrationWorld()->getManager());
	getDemonstrationWorld()->getManager()->registerWidget(d->mainWidget.get());
	getDemonstrationWorld()->getUIConfiguration()->initWidget(d->mainWidget.get());

	d->mainWidget->setTitle(L"选项菜单");
	d->mainWidget->setTitleVisible(true);
	d->mainWidget->setVerticalScrollbarWidth(20);
	gm::GMRect thumbCorner = { 0, 0, 7, 15 };
	d->mainWidget->setScrollbarThumbCorner(thumbCorner);

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
	d->mainWidget->addControl(gm::GMControlScrollBar::createControl(
		d->mainWidget.get(),
		530,
		20,
		20,
		200,
		false,
		thumbCorner
	));

	gm::GMControlButton* button = nullptr;
	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"返回主菜单",
		10,
		top,
		500,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		backToEntrance();
	});

	d->mainWidget->addControl(gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		L"这个是一个Label控件",
		getLabelFontColor(),
		10,
		top += 40,
		500,
		30,
		false
	));

	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"这是一个Button控件",
		10,
		top += 40,
		500,
		30,
		false
	));

	gm::GMRect txtCorner = { 0, 0, 6, 8 };
	gm::GMControlTextEdit* textEdit = gm::GMControlTextEdit::createControl(
		d->mainWidget.get(),
		L"这是一个TextEdit控件，可以编辑、显示单行文字",
		10,
		top += 40,
		500,
		50,
		false,
		txtCorner
	);
	textEdit->setPadding(5, 10);
	d->mainWidget->addControl(textEdit);

	gm::GMControlTextArea* textArea = gm::GMControlTextArea::createControl(
		d->mainWidget.get(),
		L"这是一个带滚动条的TextArea控件，可以编辑、显示多行文字\r\n这里是第二行\r\n\r\nWelcome To GameMachine",
		10,
		top += 120,
		500,
		100,
		false,
		true,
		txtCorner,
		thumbCorner
	);
	textArea->setPadding(5, 10);
	d->mainWidget->addControl(textArea);

	gm::GMControlTextArea* textArea2 = gm::GMControlTextArea::createControl(
		d->mainWidget.get(),
		L"这是一个不带滚动条的TextArea控件，可以编辑、显示多行文字\r\n这里是第二行\r\n\r\nWelcome To GameMachine",
		10,
		top += 240,
		500,
		100,
		false,
		false,
		txtCorner,
		thumbCorner
	);
	textArea2->setPadding(5, 10);
	d->mainWidget->addControl(textArea2);

	d->nextControlTop = top + 40;
	d->mainWidget->setSize(600, 300);

	return d->mainWidget.get();
}