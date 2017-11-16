#include "stdafx.h"
#include "button.h"
#include <gmanimation.h>

Demo_Button::~Demo_Button()
{
	D(d);
	gm::GM_delete(d->demoWorld);
	gm::GM_delete(d->buttonAnimation);
}

void Demo_Button::init()
{
	D(d);
	Base::init();

	GM_ASSERT(!d->demoWorld);
	d->demoWorld = new gm::GMDemoGameWorld();

	// 读取边框
	gm::GMGamePackage* package = GM.getGamePackageManager();
	gm::GMBuffer buf;
	bool b = package->readFile(gm::GMPackageIndex::Textures, "border.png", &buf);
	GM_ASSERT(b);

	gm::GMImage* img = nullptr;
	gm::GMImageReader::load(buf.buffer, buf.size, &img);
	gm::ITexture* frameTexture = nullptr;
	GM.getFactory()->createTexture(img, &frameTexture);
	GM_ASSERT(frameTexture);
	gm::GMAsset border = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
	gm::GMRect textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

	gm::GMImage2DGameObject* literature = new gm::GMImage2DGameObject();
	gm::GMRect rect = { 10, 10, 400, 40 };
	literature->setGeometry(rect);
	literature->setText("This is a button");

	literature->setBorder(gm::GMImage2DBorder(
		border,
		textureGeo,
		img->getWidth(),
		img->getHeight(),
		14,
		14
	));

	literature->attachEvent(*literature, gm::GM_CONTROL_EVENT_ENUM(MouseHover), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->buttonAnimation->canStart())
		{
			d->buttonAnimation->setScaling(gm::linear_math::Vector3(2.f, 2.f, 2.f), 1.f);
			d->buttonAnimation->start();
		}
		else if (d->buttonAnimation->canResume())
		{
			d->buttonAnimation->resume();
		}
	});

	literature->attachEvent(*literature, gm::GM_CONTROL_EVENT_ENUM(MouseLeave), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->buttonAnimation->canReverse())
		{
			d->buttonAnimation->reverse();
		}
	});

	d->buttonAnimation = new gm::GMAnimation(literature);

	literature->setPaddings(10, 5, 10, 5);
	d->demoWorld->addControl(literature);
	GM_delete(img);
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
		d->buttonAnimation->update();
		d->demoWorld->renderScene();
		break;
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