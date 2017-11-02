#include "stdafx.h"
#include "sound.h"
#include <linearmath.h>
#include <gmm.h>

Demo_Sound::~Demo_Sound()
{
	D(d);
	gm::GM_delete(d->wavSource);
	gm::GM_delete(d->mp3Source);
	gm::GM_delete(d->wavFile);
	gm::GM_delete(d->mp3File);
	gm::GM_delete(d->demoWorld);
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
	bool b = package->readFile(gm::GMPackageIndex::Textures, "border.png", &buf);
	GM_ASSERT(b);

	gm::GMImage* img = nullptr;
	gm::GMImageReader::load(buf.buffer, buf.size, &img);
	gm::ITexture* frameTexture = nullptr;
	GM.getFactory()->createTexture(img, &frameTexture);
	GM_ASSERT(frameTexture);
	gm::GMAsset border = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
	gm::GMRect textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

	gm::GMRect rect = { 350, 220, 100, 300 };
	listbox->setGeometry(rect);
	listbox->setItemMargins(0, 5, 0, 0);

	{
		// 获取WAV播放器，播放源
		gm::GMBuffer mp3Buffer;
		b = package->readFile(gm::GMPackageIndex::Audio, "iidesuka.wav", &mp3Buffer);
		GM_ASSERT(b);

		gm::IAudioReader* audioReader = gmm::GMMFactory::getAudioReader();
		GM_ASSERT(!d->wavFile);
		audioReader->load(mp3Buffer, &d->wavFile);

		GM_ASSERT(!d->wavSource);
		gm::IAudioPlayer* audio = gmm::GMMFactory::getAudioPlayer();
		audio->createPlayerSource(d->wavFile, &d->wavSource);

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Play WAV");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->wavSource->play(false);
			});
		}

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Pause WAV");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->wavSource->pause();
			});
		}

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Stop WAV");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->wavSource->stop();
			});
		}
	}


	{
		// 获取MP3播放器，播放源
		gm::GMBuffer mp3Buffer;
		b = package->readFile(gm::GMPackageIndex::Audio, "gyakuten.mp3", &mp3Buffer);
		GM_ASSERT(b);

		gm::IAudioReader* audioReader = gmm::GMMFactory::getAudioReader();
		GM_ASSERT(!d->mp3File);
		audioReader->load(mp3Buffer, &d->mp3File);

		GM_ASSERT(!d->mp3Source);
		gm::IAudioPlayer* audio = gmm::GMMFactory::getAudioPlayer();
		audio->createPlayerSource(d->mp3File, &d->mp3Source);

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Play MP3");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->mp3Source->play(false);
			});
		}

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Pause MP3");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->mp3Source->pause();
			});
		}

		{
			gm::GMImage2DGameObject* item = listbox->addItem("Stop MP3");
			setupItem(item, border, textureGeo, img->getData().mip[0].width, img->getData().mip[0].height);
			item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
				d->mp3Source->stop();
			});
		}
	}

	d->demoWorld->addControl(listbox);
	GM_delete(img);
}

void Demo_Sound::event(gm::GameMachineEvent evt)
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