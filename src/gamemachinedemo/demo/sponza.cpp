#include "stdafx.h"
#include "sponza.h"
#include <gmgameobject.h>
#include <gmmodelreader.h>
#include <gmcontrols.h>
#include <gmimagebuffer.h>

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

namespace
{
	gm::GMCubeMapGameObject* createCubeMap(const gm::IRenderContext* context)
	{
		gm::GMGamePackage* pk = GM.getGamePackageManager();
		gm::GMImage* slices[6] = { nullptr };
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posx.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[0]);
			GM_ASSERT(slices[0]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negx.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[1]);
			GM_ASSERT(slices[1]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posy.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[2]);
			GM_ASSERT(slices[2]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negy.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[3]);
			GM_ASSERT(slices[3]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_posz.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[4]);
			GM_ASSERT(slices[4]);
		}
		{
			gm::GMBuffer buf;
			pk->readFile(gm::GMPackageIndex::Textures, "cubemap/cubemap_negz.jpg", &buf);
			gm::GMImageReader::load(buf.buffer, buf.size, &slices[5]);
			GM_ASSERT(slices[5]);
		}

		gm::GMCubeMapBuffer cubeMap(*slices[0], *slices[1], *slices[2], *slices[3], *slices[4], *slices[5]);
		gm::GMTextureAsset cubeMapTex;
		GM.getFactory()->createTexture(context, &cubeMap, cubeMapTex);

		for (auto slice : slices)
		{
			gm::GM_delete(slice);
		}

		return new gm::GMCubeMapGameObject(cubeMapTex);
	}

	class DefaultPhysicsWorld : public gm::GMPhysicsWorld
	{
		GM_DECLARE_ALIGNED_ALLOCATOR()

		using gm::GMPhysicsWorld::GMPhysicsWorld;

	public:
		virtual void update(gm::GMDuration dt, gm::GMGameObject* obj) override
		{
			gm::GMPhysicsObject* phy = obj->getPhysicsObject();
			static GMVec3 s_z = GMVec3(0, 0, 1);

			if (phy && !m_moved)
			{
				gm::GMMotionStates s = phy->getMotionStates();
				const auto& currentTransform = s.transform;
				GMFloat4 origin_f4;
				GetTranslationFromMatrix(currentTransform, origin_f4);
				GMVec4 origin;
				origin.setFloat4(origin_f4);

				GMVec4 initialVelocity;
				{
					// x方向，只能平移
					GMVec3 lookAt = m_args.lookAt;
					lookAt.setY(0);
					GMVec3 dir = m_args.direction;
					dir.setY(0);
					dir.setZ(0);
					GMQuat q1 = GMQuat(s_z, lookAt);
					initialVelocity = GMVec4((dir * m_args.speed * m_args.rate) * q1 * dt, 0);
				}

				{
					// yz方向
					GMVec3 lookAt = m_args.lookAt;
					GMVec3 dir = m_args.direction;
					dir.setX(0);
					GMQuat q1 = GMQuat(s_z, lookAt);
					initialVelocity = initialVelocity + GMVec4((dir * m_args.speed * m_args.rate) * q1 * dt, 0);
				}

				origin = origin + initialVelocity;
				s.transform = Translate(origin);
				phy->setMotionStates(s);
				m_moved = true;
			}
		}

		virtual void applyMove(gm::GMPhysicsObject* phy, const gm::GMPhysicsMoveArgs& args) override
		{
			m_args = args;
			m_moved = false;
		}

	private:
		gm::GMPhysicsMoveArgs m_args;
		bool m_moved = true;
	};
}

void Demo_Sponza::init()
{
	Base::init();
	createMenu();
	createObject();
}

void Demo_Sponza::createMenu()
{
	D(d);
	D_BASE(db, DemoHandler);
	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMWidget* widget = createDefaultWidget();
	auto top = getClientAreaTop();

	widget->addControl(d->cpu = gm::GMControlLabel::createControl(
		widget,
		L"裁剪：开",
		GMVec4(1, 1, 1, 1),
		10,
		top,
		250,
		30,
		false
	));

	gm::GMControlButton* button = nullptr;
	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"关闭裁剪",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->sponza)
		{
			d->sponza->setCullOption(gm::GMGameObjectCullOption::None);
			d->cpu->setText(L"裁剪：关");
		}
	});

	widget->addControl(button = gm::GMControlButton::createControl(
		widget,
		L"打开裁剪",
		10,
		top += 40,
		250,
		30,
		false
	));

	connect(*button, GM_SIGNAL(gm::GMControlButton, click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (d->sponza)
		{
			d->sponza->setCullOption(gm::GMGameObjectCullOption::AABB);
			d->cpu->setText(L"裁剪：开");
		}
	});

	widget->setSize(widget->getSize().width, top + 40);
}

void Demo_Sponza::createObject()
{
	D(d);
	D_BASE(db, Base);
	
	gm::GMModelLoadSettings loadSettings(
		"sponza/sponza.obj",
		getDemoWorldReference()->getContext()
	);

	gm::GMAsset models;
	gm::GMModelReader::load(loadSettings, models);
	gm::GMAsset asset = getDemoWorldReference()->getAssets().addAsset(models);
	d->sponza = new gm::GMGameObject(asset);
	d->sponza->setCullOption(gm::GMGameObjectCullOption::AABB);
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sponza", d->sponza);

	d->skyObject = createCubeMap(getDemoWorldReference()->getContext());
	d->skyObject->setScaling(Scale(GMVec3(1500, 1500, 1500)));
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sky", d->skyObject);

	d->sprite = new gm::GMSpriteGameObject(10, getDemoWorldReference()->getContext()->getEngine()->getCamera());
	d->sprite->setMoveSpeed(GMVec3(193));
	d->sprite->setPhysicsObject(new gm::GMPhysicsObject());
	asDemoGameWorld(getDemoWorldReference())->addObject(L"sprite", d->sprite);
	gm::GMPhysicsWorld* pw = new DefaultPhysicsWorld(getDemoWorldReference().get());
}

void Demo_Sponza::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Activate:
	{
		if (d->activate)
		{
			const static gm::GMfloat mouseSensitivity = 0.25f;
			gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
			gm::IMouseState& mouseState = inputManager->getMouseState();
			auto ms = mouseState.state();
			if (d->sprite)
				d->sprite->look(Radian(-ms.deltaY * mouseSensitivity), Radian(-ms.deltaX * mouseSensitivity));

			gm::IKeyboardState& kbState = inputManager->getKeyboardState();
			if (kbState.keyTriggered(gm::GM_ASCIIToKey('R')))
				setMouseTrace(!d->mouseTrace);
		}

		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
		static gm::GMfloat mouseSensitivity = 0.25f;
		static gm::GMfloat joystickSensitivity = 0.0003f;

		gm::IKeyboardState& kbState = inputManager->getKeyboardState();
		gm::IJoystickState& joyState = inputManager->getJoystickState();
		gm::IMouseState& mouseState = inputManager->getMouseState();

		gm::GMJoystickState state = joyState.state();
		GMVec3 direction(0);
		if (d->sprite)
		{
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
		}
		break;
	}
	case gm::GameMachineHandlerEvent::Update:
	{
		gm::GMfloat dt = GM.getRunningStates().lastFrameElpased;
		getDemoWorldReference()->updateGameWorld(dt);
		getDemonstrationWorld()->getContext()->getEngine()->setCamera(d->sprite->getCamera());
		break;
	}
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Sponza::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookDirection = Normalize(GMVec3(0, 0, 1));
	lookAt.position = GMVec3(0, 10, 0);
	camera.lookAt(lookAt);
}

void Demo_Sponza::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::PointLight, &light);
			GM_ASSERT(light);
			gm::GMfloat ambientIntensity[] = { .2f, .2f, .2f };
			gm::GMfloat diffuseIntensity[] = { 1.f, 1.f, 1.f };
			light->setLightAttribute3(gm::GMLight::AmbientIntensity, ambientIntensity);
			light->setLightAttribute3(gm::GMLight::DiffuseIntensity, diffuseIntensity);
			light->setLightAttribute(gm::GMLight::SpecularIntensity, 1.f);
			light->setLightAttribute(gm::GMLight::AttenuationLinear, .0005f);

			gm::GMfloat lightPos[] = { 0.f, 500.f, 0.f };
			light->setLightAttribute3(gm::GMLight::Position, lightPos);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Sponza::onActivate()
{
	D(d);
	d->activate = true;
	setMouseTrace(true);
	Base::onActivate();
}

void Demo_Sponza::onDeactivate()
{
	D(d);
	d->activate = false;
	setMouseTrace(false);
	Base::onDeactivate();
}

void Demo_Sponza::setMouseTrace(bool enabled)
{
	D(d);
	gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputManager();
	gm::IMouseState& mouseState = inputManager->getMouseState();
	d->mouseTrace = enabled;

	// 鼠标跟踪开启时，detecting mode也开启，每一帧将返回窗口中心，以获取鼠标移动偏量
	mouseState.setDetectingMode(d->mouseTrace);
}