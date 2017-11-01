#include "stdafx.h"
#include "particles1.h"
#include <linearmath.h>

Demo_Particles1::~Demo_Particles1()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Particles1::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();

	gm::linear_math::Quaternion start, end;
	start.setRotation(gm::linear_math::Vector3(0, 0, 1), 0.f);
	end.setRotation(gm::linear_math::Vector3(0, 0, 1), 5.f);
	gm::GMRadiusParticlesEmitter::create(
		50,
		gm::GMParticlePositionType::RespawnAtEmitterPosition,
		1,
		.01f,
		.1f,
		gm::linear_math::Vector3(0, 0, 1),
		3.14159f,
		gm::linear_math::Vector3(0, 0, 0),
		gm::linear_math::Vector3(1, 0, 0),
		gm::linear_math::Vector4(1, 0, 0, 1),
		gm::linear_math::Vector4(0, 1, 0, 0),
		start,
		end,
		0.1f,
		1.f,
		gm::GMParticlesEmitter::InfiniteEmissionTimes,
		&d->emitter
		);
	d->demoWorld->appendObject("particles", d->emitter);
}

void Demo_Particles1::event(gm::GameMachineEvent evt)
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
