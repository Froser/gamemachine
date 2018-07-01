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
	case gm::GameMachineHandlerEvent::Simulate:
		getDemoWorldReference()->simulateGameWorld();
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

	gm::GMControlButton* button = nullptr;
	d->mainWidget->addButton(
		L"返回主菜单",
		10,
		top,
		500,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		backToEntrance();
	});

	d->mainWidget->addLabel(
		L"这个是一个Label控件",
		getLabelFontColor(),
		10,
		top += 40,
		500,
		30,
		false,
		nullptr
	);

	d->mainWidget->addButton(
		L"这是一个Button控件",
		10,
		top += 40,
		500,
		30,
		false,
		&button
	);

	gm::GMRect txtCorner = { 0, 0, 6, 8 };
	gm::GMControlTextEdit* textEdit = nullptr;
	d->mainWidget->addTextEdit(
		L"这是一个TextEdit控件，可以编辑、显示单行文字",
		10,
		top += 40,
		500,
		50,
		false,
		txtCorner,
		&textEdit
	);
	textEdit->setPadding(5, 10);

	gm::GMControlTextArea* textArea = nullptr;
	d->mainWidget->addTextArea(
		L"这是一个TextArea控件，可以编辑、显示多行文字\r\n这里是第二行\r\n\r\nWelcome To GameMachine",
		10,
		top += 60,
		500,
		100,
		false,
		txtCorner,
		&textArea
	);
	textEdit->setPadding(5, 10);

	d->nextControlTop = top + 40;
	d->mainWidget->setSize(800, 600);

	return d->mainWidget.get();
}