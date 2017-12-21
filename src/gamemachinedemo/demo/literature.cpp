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
	gm::GMRect rect = { 200, 220, 400, 190 };
	literature->setGeometry(rect);
	literature->setText(
		"This is a [color=#ffbbff]text[color=#ffffff] demo. It shows you how to render ABC,xyz,etc.[n]"
		"[color=#ff0000]red[n]"
		"[color=#00ff00]green[n]"
		"[color=#0000ff]blue[n]"
		"[color=#ff00FF]magenta[n][color=#ffffff]"
		"[size=20]20Points Font Size[n]"
		"[size=10]10Points Font Size[n]"
		"[n]"
		"[size=25][n]Let's try some 'overflow'"
	);

	literature->setBorder(gm::GMImage2DBorder(
		border,
		textureGeo,
		img->getWidth(),
		img->getHeight(),
		14,
		14
	));
	literature->setPaddings(10, 17, 10, 15);
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