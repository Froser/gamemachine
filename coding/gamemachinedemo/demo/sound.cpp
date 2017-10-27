#include "stdafx.h"
#include "sound.h"
#include <linearmath.h>
#include <gmm.h>

Demo_Sound::~Demo_Sound()
{
	D(d);
	gm::GM_delete(d->demoWorld);
	gm::GM_delete(d->audioSource);
}

void Demo_Sound::init()
{
	D(d);
	Base::init();

	GM_ASSERT(!d->demoWorld);
	d->demoWorld = new gm::GMDemoGameWorld();

	// 读取边框
	gm::GMListbox2DGameObject* listbox = new gm::GMListbox2DGameObject();
	gm::GMGamePackage* package = GM.getGamePackageManager();
	gm::GMBuffer buf;
	package->readFile(gm::GMPackageIndex::Textures, "border.png", &buf);
	gm::GMImage* img = nullptr;
	gm::GMImageReader::load(buf.buffer, buf.size, &img);
	gm::ITexture* frameTexture = nullptr;
	GM.getFactory()->createTexture(img, &frameTexture);
	GM_ASSERT(frameTexture);
	gm::GMAsset border = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
	gm::GMRect textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

	gm::GMRect rect = { 350, 300, 100, 300 };
	listbox->setGeometry(rect);
	listbox->setItemMargins(0, 5, 0, 0);

	// 获取播放器，播放源
	gm::GMBuffer musicBuffer;
	package->readFile(gm::GMPackageIndex::Audio, "gyakuten.mp3", &musicBuffer);

	gm::IAudioReader* audioReader = nullptr;
	gmm::GMMFactory::createAudioReader(&audioReader);
	GM_ASSERT(audioReader);

	GM_ASSERT(!d->file);
	audioReader->load(musicBuffer, &d->file);
	GM_delete(audioReader);

	GM_ASSERT(!d->audioSource);
	gm::IAudioPlayer* audio = GM.getAudioPlayer();
	audio->createPlayerSource(d->file, &d->audioSource);
	
	{
		gm::GMImage2DGameObject* item = listbox->addItem("Play music");
		setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
		item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->audioSource->play(true);
		});
	}

	{
		gm::GMImage2DGameObject* item = listbox->addItem("Pause music");
		setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
		item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->audioSource->pause();
		});
	}


	{
		gm::GMImage2DGameObject* item = listbox->addItem("Stop music");
		setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
		item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->audioSource->stop();
		});
	}

	d->demoWorld->addControl(listbox);
	GM_delete(img);
}

void Demo_Sound::event(gm::GameMachineEvent evt)
{
	D(d);
	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		d->demoWorld->simulateGameWorld();
		d->demoWorld->notifyControls();
		break;
	case gm::GameMachineEvent::Render:
	{
		d->demoWorld->renderScene();
		break;
	}
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

void Demo_Sound::setupItem(gm::GMImage2DGameObject* item, gm::GMAsset border, const gm::GMRect& textureGeo, gm::GMint imgWidth, gm::GMint imgHeight)
{
	item->setBorder(gm::GMImage2DBorder(
		border,
		textureGeo,
		imgWidth,
		imgHeight,
		14,
		14
	));
	item->setHeight(30);
	item->setPaddings(10, 5, 10, 5);
}