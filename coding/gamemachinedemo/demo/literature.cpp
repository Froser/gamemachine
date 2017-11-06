#include "stdafx.h"
#include "literature.h"
#include <linearmath.h>
#include <gmm.h>

Demo_Literature::~Demo_Literature()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Literature::init()
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
	gm::GMRect rect = { 200, 220, 400, 200 };
	literature->setGeometry(rect);
	literature->setText(
		"This is a [color=#ffbbff]text[color=#ffffff] demo. It shows you how to render ABC,xyz,etc.[n]"
		"[color=#ff0000]red[n]"
		"[color=#00ff00]green[n]"
		"[color=#0000ff]blue[n]"
		"[color=#ff00FF]magenta[n]"
	);
	setupItem(literature, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
	d->demoWorld->addControl(literature);
	GM_delete(img);
}

void Demo_Literature::event(gm::GameMachineEvent evt)
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
		d->demoWorld->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
		break;
	case gm::GameMachineEvent::Deactivate:
		break;
	case gm::GameMachineEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Literature::setupItem(gm::GMImage2DGameObject* item, gm::GMAsset border, const gm::GMRect& textureGeo, gm::GMint imgWidth, gm::GMint imgHeight)
{
	item->setBorder(gm::GMImage2DBorder(
		border,
		textureGeo,
		imgWidth,
		imgHeight,
		14,
		14
	));
	item->setPaddings(10, 5, 10, 5);
}