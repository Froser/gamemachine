#include "stdafx.h"
#include "sound.h"
#include <linearmath.h>
#include <gmm.h>
#include <gmwidget.h>
#include <gmcontrols.h>

Demo_Sound::~Demo_Sound()
{
	D(d);
	gm::GM_delete(d->wavSource);
	gm::GM_delete(d->mp3Source);
	gm::GM_delete(d->wavFile);
	gm::GM_delete(d->mp3File);
}

void Demo_Sound::onActivate()
{
	D(d);
	Base::onActivate();
}

void Demo_Sound::onDeactivate()
{
	D(d);
	Base::onDeactivate();
}

void Demo_Sound::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();

	GM_ASSERT(!getDemoWorldReference());
	getDemoWorldReference() = new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext());

	gm::GMGamePackage* package = GM.getGamePackageManager();

	// 准备音频
	{
		// 获取WAV播放器，播放源
		gm::GMBuffer mp3Buffer;
		bool b = package->readFile(gm::GMPackageIndex::Audio, "iidesuka.wav", &mp3Buffer);
		GM_ASSERT(b);

		gm::IAudioReader* audioReader = gmm::GMMFactory::getAudioReader();
		GM_ASSERT(!d->wavFile);
		audioReader->load(mp3Buffer, &d->wavFile);

		GM_ASSERT(!d->wavSource);
		gm::IAudioPlayer* audio = gmm::GMMFactory::getAudioPlayer();
		audio->createPlayerSource(d->wavFile, &d->wavSource);
	}

	{
		// 获取MP3播放器，播放源
		gm::GMBuffer mp3Buffer;
		bool b = package->readFile(gm::GMPackageIndex::Audio, "gyakuten.mp3", &mp3Buffer);
		GM_ASSERT(b);

		gm::IAudioReader* audioReader = gmm::GMMFactory::getAudioReader();
		GM_ASSERT(!d->mp3File);
		audioReader->load(mp3Buffer, &d->mp3File);

		GM_ASSERT(!d->mp3Source);
		gm::IAudioPlayer* audio = gmm::GMMFactory::getAudioPlayer();
		audio->createPlayerSource(d->mp3File, &d->mp3Source);
	}

	createDefaultWidget();
	gm::GMWidget* widget = getWidget();
	widget->setSize(300, 300);

	gm::GMControlButton* button = nullptr;
	widget->addButton(
		"Play WAV",
		10,
		40,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->play(false);
	});

	widget->addButton(
		"Pause WAV",
		10,
		80,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->pause();
	});

	widget->addButton(
		"Stop WAV",
		10,
		120,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->stop();
	});

	widget->addButton(
		"Play MP3",
		10,
		160,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->play(false);
	});

	widget->addButton(
		"Pause MP3",
		10,
		200,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->stop();
	});

	widget->addButton(
		"Stop MP3",
		10,
		240,
		250,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->stop();
	});
}

void Demo_Sound::event(gm::GameMachineHandlerEvent evt)
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