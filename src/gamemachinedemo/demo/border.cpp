#include "stdafx.h"
#include "border.h"
#include <linearmath.h>
#include <gmm.h>

void Demo_Border::init()
{
	D(d);
	Base::init();

	GM_ASSERT(!getDemoWorldReference());
	getDemoWorldReference() = new gm::GMDemoGameWorld();

	// 读取边框
	gm::GMGamePackage* package = GM.getGamePackageManager();
	gm::GMBuffer buf;
	bool b = package->readFile(gm::GMPackageIndex::Textures, "frame.png", &buf);
	GM_ASSERT(b);

	gm::GMImage* img = nullptr;
	gm::GMImageReader::load(buf.buffer, buf.size, &img);
	gm::ITexture* frameTexture = nullptr;
	GM.getFactory()->createTexture(img, &frameTexture);
	GM_ASSERT(frameTexture);
	gm::GMAsset border = getDemoWorldReference()->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
	gm::GMRect textureGeo = { 96, 96, 96, 96}; //截取的纹理位置

	gm::GMImage2DGameObject* borderObject = new gm::GMImage2DGameObject();
	gm::GMRect rect = { 10, 10, 400, 200 };
	borderObject->setGeometry(rect);
	borderObject->setBorder(gm::GMImage2DBorder(
		border,
		textureGeo,
		img->getWidth(),
		img->getHeight(),
		14,
		14
	));
	getDemoWorldReference()->addControl(borderObject);
	GM_delete(img);
}

void Demo_Border::event(gm::GameMachineHandlerEvent evt)
{
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