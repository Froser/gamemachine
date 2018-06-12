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
	gm::GM_delete(d->widget);
}

void Demo_Sound::onActivate()
{
	D(d);
	Base::onActivate();
	d->widget->setVisible(true);
}

void Demo_Sound::onDeactivate()
{
	D(d);
	Base::onDeactivate();
	d->widget->setVisible(false);
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

	d->widget = new gm::GMWidget(getDemonstrationWorld()->getManager());
	{
		gm::GMRect rc = { 0, 0, 136, 54 };
		d->widget->addArea(gm::GMTextureArea::ButtonArea, rc);
	}
	{
		gm::GMRect rc = { 136, 0, 116, 54 };
		d->widget->addArea(gm::GMTextureArea::ButtonFillArea, rc);
	}

	d->widget->setKeyboardInput(true);

	gm::GMControlButton* button = nullptr;
	d->widget->addButton(
		"Play WAV",
		10,
		100,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->play(false);
	});

	d->widget->addButton(
		"Pause WAV",
		10,
		150,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->pause();
	});

	d->widget->addButton(
		"Stop WAV",
		10,
		200,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->wavSource->stop();
	});

	d->widget->addButton(
		"Play MP3",
		10,
		250,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->play(false);
	});

	d->widget->addButton(
		"Pause MP3",
		10,
		300,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->stop();
	});

	d->widget->addButton(
		"Stop MP3",
		10,
		350,
		600,
		30,
		false,
		&button
	);
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->mp3Source->stop();
	});

	d->widget->setVisible(false);
	getDemoWorldReference()->getContext()->getWindow()->addWidget(d->widget);
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