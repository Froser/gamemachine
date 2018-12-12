#include "stdafx.h"
#include "quake3_bsp.h"
#include <linearmath.h>
#include <gmbspgameworld.h>
#include <gmbspfactory.h>
#include <gmwidget.h>
#include <gmcontrols.h>

void Demo_Quake3_BSP::onActivate()
{
	D(d);
	Base::onActivate();
	setMouseTrace(true);
}

void Demo_Quake3_BSP::onDeactivate()
{
	D(d);
	D_BASE(db, Base);
	Base::onDeactivate();
	db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);
	setMouseTrace(false);
}

void Demo_Quake3_BSP::setLookAt()
{
	// 设置一个默认视角
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);
	getDemonstrationWorld()->getContext()->getEngine()->getCamera().updateViewMatrix();
}

void Demo_Quake3_BSP::setDefaultLights()
{
	D(d);
	if (isInited())
		d->world->setDefaultLights();
}

void Demo_Quake3_BSP::setMouseTrace(bool enabled)
{
	D(d);
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseTrace = enabled;

	// 鼠标跟踪开启时，detecting mode也开启，每一帧将返回窗口中心，以获取鼠标移动偏量
	mouseState.setDetectingMode(d->mouseTrace);
}

void Demo_Quake3_BSP::init()
{
	D(d);
	D_BASE(db, Base);
	Base::init();
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();

	gm::GMBSPFactory::createBSPGameWorld(db->parentDemonstrationWorld->getContext(), "gv.bsp", &d->world);
	db->demoWorld.reset(d->world);
	d->sprite = d->world->getSprite();

	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();
	widget->addControl(gm::GMControlLabel::createControl(
		widget,
		L"W、S、A、D或XBOX手柄移动",
		getLabelFontColor(),
		10,
		top,
		250,
		30,
		false
	));
	widget->addControl(gm::GMControlLabel::createControl(
		widget,
		L"Space跳跃",
		getLabelFontColor(),
		10,
		top += 20,
		250,
		30,
		false
	));
	widget->addControl(gm::GMControlLabel::createControl(
		widget,
		L"R显示/隐藏鼠标",
		getLabelFontColor(),
		10,
		top += 20,
		250,
		30,
		false
	));

	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"开启/关闭计算BSP面",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->world->setRenderConfig(gm::GMBSPRenderConfigs::CalculateFace_Bool, !d->world->getRenderConfig(gm::GMBSPRenderConfigs::CalculateFace_Bool).toBool());
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"开启/关闭只绘制天空",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->world->setRenderConfig(gm::GMBSPRenderConfigs::DrawSkyOnly_Bool, !d->world->getRenderConfig(gm::GMBSPRenderConfigs::DrawSkyOnly_Bool).toBool());
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"开启/关闭只绘制Lightmap",
		10,
		top += 40,
		250,
		30,
		false
	));
	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		db->debugConfig.set(gm::GMDebugConfigs::DrawLightmapOnly_Bool, !db->debugConfig.get(gm::GMDebugConfigs::DrawLightmapOnly_Bool).toBool());
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Quake3_BSP::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	D_BASE(db, Base);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Update:
	{
		d->world->updateGameWorld(GM.getRunningStates().lastFrameElpased);
		// 更新Camera
		getDemonstrationWorld()->getContext()->getEngine()->setCamera(d->sprite->getCamera());
		break;
	}
	case gm::GameMachineHandlerEvent::Render:
	{
		d->world->renderScene();
		break;
	}
	case gm::GameMachineHandlerEvent::Activate:
	{
		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
		static gm::GMfloat mouseSensitivity = 0.25f;
		static gm::GMfloat joystickSensitivity = 0.0003f;

		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		gm::IJoystickState& joyState = inputManager->getJoystickState();
		gm::IMouseState& mouseState = inputManager->getMouseState();

		gm::GMJoystickState state = joyState.joystickState();
		GMVec3 direction(0);

		if (kbState.keydown(gm::GM_ASCIIToKey('A')))
			d->sprite->action(gm::GMMovement::Move, GMVec3(-1, 0, 0));
		if (kbState.keydown(gm::GM_ASCIIToKey('D')))
			d->sprite->action(gm::GMMovement::Move, GMVec3(1, 0, 0));
		if (kbState.keydown(gm::GM_ASCIIToKey('S')))
			d->sprite->action(gm::GMMovement::Move, GMVec3(0, 0, -1));
		if (kbState.keydown(gm::GM_ASCIIToKey('W')))
			d->sprite->action(gm::GMMovement::Move, GMVec3(0, 0, 1));

		if (state.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, GMVec3(1, 0, 0), GMVec3(gm::GMfloat(state.thumbLX) / SHRT_MAX));
		if (state.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, GMVec3(-1, 0, 0), GMVec3(gm::GMfloat(state.thumbLX) / SHRT_MIN));
		if (state.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, GMVec3(0, 0, -1), GMVec3(gm::GMfloat(state.thumbLY) / SHRT_MIN));
		if (state.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, GMVec3(0, 0, 1), GMVec3(gm::GMfloat(state.thumbLY) / SHRT_MAX));

		if (kbState.keyTriggered(gm::GMKey_Space) || state.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER || state.buttons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			d->sprite->action(gm::GMMovement::Jump);

		if (kbState.keyTriggered(gm::GM_ASCIIToKey('V')))
			joyState.joystickVibrate(30000, 30000);
		else if (kbState.keydown(gm::GM_ASCIIToKey('C')))
			joyState.joystickVibrate(0, 0);

		if (kbState.keyTriggered(gm::GM_ASCIIToKey('I')))
			db->debugConfig.set(gm::GMDebugConfigs::RunProfile_Bool, !db->debugConfig.get(gm::GMDebugConfigs::RunProfile_Bool).toBool());

		gm::GMfloat joystickPitch = 0, joystickYaw = 0;
		if (state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRX / (
				state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			joystickYaw = -state.thumbRX * joystickSensitivity * rate;
		}
		if (state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRY / (
				state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			joystickPitch = state.thumbRY * joystickSensitivity * rate;
		}
		d->sprite->look(Radian(joystickPitch), Radian(joystickYaw));

		gm::GMMouseState ms = mouseState.mouseState();
		d->sprite->look(Radian(-ms.deltaY * mouseSensitivity), Radian(-ms.deltaX * mouseSensitivity));
		
		if (kbState.keyTriggered(gm::GM_ASCIIToKey('R')))
			setMouseTrace(!d->mouseTrace);

		if (kbState.keyTriggered(gm::GM_ASCIIToKey('0')))
			db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, 0);

		for (gm::GMint32 i = 0; i < 8; ++i)
		{
			if (kbState.keyTriggered(gm::GM_ASCIIToKey('1' + (gm::GMint32)i)))
				db->debugConfig.set(gm::GMDebugConfigs::FrameBufferIndex_I32, (gm::GMint32)i + 1);
		}
		break;
	}
	}
}
