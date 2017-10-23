#include "stdafx.h"
#include "quake3_bsp.h"
#include <linearmath.h>
#include <gmbspgameworld.h>
#include <gmbspfactory.h>

Demo_Quake3_BSP::~Demo_Quake3_BSP()
{
	D(d);
	gm::GM_delete(d->world);
}

void Demo_Quake3_BSP::onActivate()
{
	D(d);
	GM.getCamera().synchronizeLookAt();

	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseEnabled = true;
	mouseState.setMouseEnable(d->mouseEnabled);
}

void Demo_Quake3_BSP::init()
{
	D(d);
	Base::init();
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	inputManager->getKeyboardState().setIMEState(false);

	gm::GMBSPFactory::createBSPGameWorld("gv.bsp", &d->world);
	d->sprite = static_cast<gm::GMSpriteGameObject*> (
		const_cast<gm::GMGameObject*> (
			*(d->world->getGameObjects(gm::GMGameObjectType::Sprite).begin())
			)
		);
}

void Demo_Quake3_BSP::event(gm::GameMachineEvent evt)
{
	D(d);
	switch (evt)
	{
	case gm::GameMachineEvent::Simulate:
	{
		d->world->simulateGameWorld();
		// 更新Camera
		GM.getCamera().synchronize(d->sprite);
		break;
	}
	case gm::GameMachineEvent::Render:
	{
		GM.getCamera().synchronizeLookAt();
		d->world->renderScene();
		break;
	}
	case gm::GameMachineEvent::Activate:
	{
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		static gm::GMfloat mouseSensitivity = 0.25f;
		static gm::GMfloat joystickSensitivity = 0.0003f;

		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		gm::IJoystickState& joyState = inputManager->getJoystickState();
		gm::IMouseState& mouseState = inputManager->getMouseState();

		gm::GMMovement moveTag = MC_NONE;
		gm::GMMoveRate rate;
		gm::GMJoystickState state = joyState.joystickState();

		if (kbState.keydown('A'))
			moveTag |= MC_LEFT;
		if (state.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			moveTag |= MC_LEFT;
			rate.setMoveRate(MC_LEFT, gm::GMfloat(state.thumbLX) / SHRT_MIN);
		}

		if (kbState.keydown('D'))
			moveTag |= MC_RIGHT;
		if (state.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			moveTag |= MC_RIGHT;
			rate.setMoveRate(MC_RIGHT, gm::GMfloat(state.thumbLX) / SHRT_MAX);
		}

		if (kbState.keydown('S'))
			moveTag |= MC_BACKWARD;
		if (state.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			moveTag |= MC_BACKWARD;
			rate.setMoveRate(MC_BACKWARD, gm::GMfloat(state.thumbLY) / SHRT_MIN);
		}

		if (kbState.keydown('W'))
			moveTag |= MC_FORWARD;
		if (state.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			moveTag |= MC_FORWARD;
			rate.setMoveRate(MC_FORWARD, gm::GMfloat(state.thumbLY) / SHRT_MAX);
		}

		if (kbState.keyTriggered(VK_SPACE) || state.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER || state.buttons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			moveTag |= MC_JUMP;

		if (kbState.keyTriggered('V'))
			joyState.joystickVibrate(30000, 30000);
		else if (kbState.keydown('C'))
			joyState.joystickVibrate(0, 0);

		if (kbState.keyTriggered('N'))
			GMSetDebugState(DRAW_NORMAL, (GMGetDebugState(DRAW_NORMAL) + 1) % gm::GMStates_DebugOptions::DRAW_NORMAL_END);
		if (kbState.keyTriggered('M'))
			GMSetDebugState(DRAW_LIGHTMAP_ONLY, !GMGetDebugState(DRAW_LIGHTMAP_ONLY));
		if (kbState.keyTriggered('I'))
			GMSetDebugState(RUN_PROFILE, !GMGetDebugState(RUN_PROFILE));

		if (state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRX / (
				state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			d->sprite->lookRight(state.thumbRX * joystickSensitivity * rate);
		}
		if (state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRY / (
				state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			d->sprite->lookUp(state.thumbRY * joystickSensitivity * rate);
		}

		gm::GMMouseState ms = mouseState.mouseState();
		d->sprite->lookUp(-ms.deltaY * mouseSensitivity);
		d->sprite->lookRight(ms.deltaX * mouseSensitivity);
		d->sprite->action(moveTag, rate);

		if (kbState.keyTriggered('P'))
			GMSetDebugState(CALCULATE_BSP_FACE, !GMGetDebugState(CALCULATE_BSP_FACE));
		if (kbState.keyTriggered('L'))
			GMSetDebugState(POLYGON_LINE_MODE, !GMGetDebugState(POLYGON_LINE_MODE));
		if (kbState.keyTriggered('O'))
			GMSetDebugState(DRAW_ONLY_SKY, !GMGetDebugState(DRAW_ONLY_SKY));
		if (kbState.keyTriggered('R'))
			mouseState.setMouseEnable(d->mouseEnabled = !d->mouseEnabled);

		if (kbState.keyTriggered('0'))
			GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, 0);
		GM_FOREACH_ENUM_CLASS(i, gm::GBufferGeometryType::Position, gm::GBufferGeometryType::EndOfGeometryType)
		{
			if (kbState.keyTriggered('1' + (gm::GMint)i))
				GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, (gm::GMint)i + 1);
		}
		break;
	}
	}
}
