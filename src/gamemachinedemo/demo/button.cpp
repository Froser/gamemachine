#include "stdafx.h"
#include "button.h"
#include <gmanimation.h>

Demo_Button::~Demo_Button()
{
	D(d);
	for (gm::GMint i = 0; i < GM_array_size(d->buttons); ++i)
	{
		GM_delete(d->buttons[i].animation);
	}
}

void Demo_Button::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	GM_ASSERT(!getDemoWorldReference());
	getDemoWorldReference() = new gm::GMDemoGameWorld();
	initBorder();
}

void Demo_Button::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	D_BASE(db, Base);
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
	{
		for (gm::GMint i = 0; i < GM_array_size(d->buttons); ++i)
		{
			if (d->buttons[i].animation)
				d->buttons[i].animation->update();
		}
		getDemoWorldReference()->renderScene();
		break;
	}
	case gm::GameMachineHandlerEvent::Activate:
		getDemoWorldReference()->notifyControls();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Button::initBorder()
{
	D(d);
	D_BASE(db, Base);
	if (!d->border.asset)
	{
		// 读取边框
		gm::GMGamePackage* package = GM.getGamePackageManager();
		gm::GMBuffer buf;
		bool b = package->readFile(gm::GMPackageIndex::Textures, "border.png", &buf);
		GM_ASSERT(b);

		gm::GMImage* border = nullptr;
		gm::GMImageReader::load(buf.buffer, buf.size, &border);
		gm::ITexture* frameTexture = nullptr;
		GM.getFactory()->createTexture(border, &frameTexture);
		GM_ASSERT(frameTexture);
		d->border = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
		d->textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

		{
			gm::GMImage2DGameObject* button = newButton<0>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				animation->setScaling(GMVec3(1.2f, 1.2f, 1.2f));
			}).button;
			gm::GMRect rect = { 50, 30, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a scaling button");
			getDemoWorldReference()->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<1>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				animation->setTranslation(65, 100);
			}).button;
			gm::GMRect rect = { 50, 100, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a translation button");
			getDemoWorldReference()->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<2>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				GMQuat q = Rotate(3.14f / 6.f, GMVec3(1, 1, 1));
				animation->setRotation(q);
			}).button;
			gm::GMRect rect = { 50, 170, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a rotation button");
			getDemoWorldReference()->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<3>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				GMQuat q = Rotate(3.14f / 6.f, GMVec3(1, 1, 1));
				animation->setRotation(q);
				animation->setScaling(GMVec3(1.2f, 1.2f, 1.2f));
			}).button;
			gm::GMRect rect = { 50, 240, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a mixed button");
			getDemoWorldReference()->addControl(button);
		}

		GM_delete(border);
	}
}

template <gm::GMint idx>
Button& Demo_Button::newButton(gm::GMImage* border, const AnimationAction& onHoverAction)
{
	D(d);
	GM_STATIC_ASSERT(idx < GM_array_size(d->buttons), "overflow");
	gm::GMImage2DGameObject* button = new gm::GMImage2DGameObject();
	gm::GMControlGameObjectAnimation* animation = new gm::GMControlGameObjectAnimation(button);
	d->buttons[idx].button = button;
	d->buttons[idx].animation = animation;
	d->buttons[idx].animation->setDuration(.25f);

	button->setPaddings(10, 10, 10, 5);

	button->setBorder(gm::GMImage2DBorder(
		d->border,
		d->textureGeo,
		border->getWidth(),
		border->getHeight(),
		14,
		14
	));

	button->attachEvent(*button, gm::GM_CONTROL_EVENT_ENUM(MouseHover), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (animation->canStart())
		{
			onHoverAction(animation);
			animation->start();
		}
		else if (animation->canResume())
		{
			animation->resume();
		}
	});

	button->attachEvent(*button, gm::GM_CONTROL_EVENT_ENUM(MouseLeave), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (animation->canReverse())
		{
			animation->reverse();
		}
	});

	return d->buttons[idx];
}