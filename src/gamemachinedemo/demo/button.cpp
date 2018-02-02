#include "stdafx.h"
#include "button.h"
#include <gmanimation.h>

Demo_Button::~Demo_Button()
{
	D(d);
	gm::GM_delete(d->demoWorld);

	for (gm::GMint i = 0; i < GM_array_size(d->buttons); ++i)
	{
		GM_delete(d->buttons[i].animation);
	}
}

void Demo_Button::init()
{
	D(d);
	Base::init();

	GM_ASSERT(!d->demoWorld);
	d->demoWorld = new gm::GMDemoGameWorld();
	initBorder();
}

void Demo_Button::event(gm::GameMachineEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		d->demoWorld->simulateGameWorld();
		break;
	case gm::GameMachineEvent::Render:
	{
		for (gm::GMint i = 0; i < GM_array_size(d->buttons); ++i)
		{
			if (d->buttons[i].animation)
				d->buttons[i].animation->update();
		}
		d->demoWorld->renderScene();
		break;
	}
	case gm::GameMachineEvent::Activate:
		d->demoWorld->notifyControls();
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Button::initBorder()
{
	D(d);
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
		d->border = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
		d->textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

		{
			gm::GMImage2DGameObject* button = newButton<0>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				animation->setScaling(GMVec3(1.2f, 1.2f, 1.2f));
			}).button;
			gm::GMRect rect = { 50, 30, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a scaling button");
			d->demoWorld->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<1>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				animation->setTranslation(65, 100);
			}).button;
			gm::GMRect rect = { 50, 100, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a translation button");
			d->demoWorld->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<2>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				GMQuat q = glm::rotate(Identity<GMQuat>(), 3.14f / 6.f, GMVec3(1, 1, 1));
				animation->setRotation(q);
			}).button;
			gm::GMRect rect = { 50, 170, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a rotation button");
			d->demoWorld->addControl(button);
		}

		{
			gm::GMImage2DGameObject* button = newButton<3>(border, [=](gm::GMControlGameObjectAnimation* animation) {
				GMQuat q = glm::rotate(Identity<GMQuat>(), 3.14f / 6.f, GMVec3(1, 1, 1));
				animation->setRotation(q);
				animation->setScaling(GMVec3(1.2f, 1.2f, 1.2f));
			}).button;
			gm::GMRect rect = { 50, 240, 400, 40 };
			button->setGeometry(rect);
			button->setText("This is a mixed button");
			d->demoWorld->addControl(button);
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