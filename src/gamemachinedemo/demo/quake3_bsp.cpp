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
	Base::onActivate();
	setMouseTrace(true);
}

void Demo_Quake3_BSP::onDeactivate()
{
	D(d);
	Base::onDeactivate();
	GMSetDebugState(FRAMEBUFFER_VIEWER_INDEX, 0);
	setMouseTrace(false);
}

void Demo_Quake3_BSP::setLookAt()
{
	// 设置一个默认视角
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);
	GM.getCamera().synchronizeLookAt();
}

void Demo_Quake3_BSP::setDefaultLights()
{
	D(d);
	if (isInited())
		d->world->setDefaultLights();
}

void Demo_Quake3_BSP::setMouseTrace(bool enabled)
{
	// 如果开启了鼠标跟踪，将会自动隐藏鼠标
	D(d);
	if (enabled)
		GM.getCursor()->disableCursor();
	else
		GM.getCursor()->enableCursor();
	
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseTrace = enabled;

	// 鼠标跟踪开启时，detecting mode也开启，每一帧将返回窗口中心，以获取鼠标移动偏量
	mouseState.setDetectingMode(d->mouseTrace);
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
	Base::event(evt);
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

		gm::GMJoystickState state = joyState.joystickState();
		glm::vec3 direction(0);

		if (kbState.keydown('A'))
			d->sprite->action(gm::GMMovement::Move, glm::vec3(-1, 0, 0));
		if (state.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, glm::vec3(-1, 0, 0), glm::vec3(gm::GMfloat(state.thumbLX) / SHRT_MIN));

		if (kbState.keydown('D'))
			d->sprite->action(gm::GMMovement::Move, glm::vec3(1, 0, 0));
		if (state.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, glm::vec3(-1, 0, 0), glm::vec3(gm::GMfloat(state.thumbLX) / SHRT_MAX));

		if (kbState.keydown('S'))
			d->sprite->action(gm::GMMovement::Move, glm::vec3(0, 0, -1));
		if (state.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, glm::vec3(0, 0, -1), glm::vec3(gm::GMfloat(state.thumbLY) / SHRT_MIN));

		if (kbState.keydown('W'))
			d->sprite->action(gm::GMMovement::Move, glm::vec3(0, 0, 1));
		if (state.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			d->sprite->action(gm::GMMovement::Move, glm::vec3(0, 0, 1), glm::vec3(gm::GMfloat(state.thumbLY) / SHRT_MAX));

		if (kbState.keyTriggered(VK_SPACE) || state.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER || state.buttons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			d->sprite->action(gm::GMMovement::Jump);

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

		gm::GMfloat joystickPitch = 0, joystickYaw = 0;
		if (state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRX / (
				state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			joystickPitch = state.thumbRX * joystickSensitivity * rate;
		}
		if (state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			gm::GMfloat rate = (gm::GMfloat)state.thumbRY / (
				state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
				SHRT_MIN :
				SHRT_MAX);

			joystickYaw = state.thumbRY * joystickSensitivity * rate;
		}
		d->sprite->look(glm::radians(joystickPitch), glm::radians(joystickYaw));

		gm::GMMouseState ms = mouseState.mouseState();
		d->sprite->look(glm::radians(-ms.deltaY * mouseSensitivity), glm::radians(ms.deltaX * mouseSensitivity));
		
		if (kbState.keyTriggered('P'))
			GMSetDebugState(CALCULATE_BSP_FACE, !GMGetDebugState(CALCULATE_BSP_FACE));
		if (kbState.keyTriggered('O'))
			GMSetDebugState(DRAW_ONLY_SKY, !GMGetDebugState(DRAW_ONLY_SKY));
		if (kbState.keyTriggered('R'))
			setMouseTrace(!d->mouseTrace);

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
