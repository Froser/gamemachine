#include "stdafx.h"
#include "particles2.h"
#include <linearmath.h>

Demo_Particles2::~Demo_Particles2()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Particles2::init()
{
	D(d);
	Base::init();

	// 创建对象
	d->demoWorld = new gm::GMDemoGameWorld();

	glm::quat start = glm::rotate(glm::identity<glm::quat>(), 0.f, glm::vec3(0, 0, 1)),
		end = glm::rotate(glm::identity<glm::quat>(), 5.f, glm::vec3(0, 0, 1));
	gm::GMLerpParticleEmitter::create(
		50,
		gm::GMParticlePositionType::FollowEmitter,
		1,
		.01f,
		.1f,
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(-1.414f / 2, -1.414f / 2, 0),
		glm::vec4(1, 0, 0, 1),
		glm::vec4(0, 1, 0, 0),
		start,
		end,
		0.1f,
		1.f,
		gm::GMParticlesEmitter::InfiniteEmissionTimes,
		&d->emitter
		);
	d->demoWorld->addObject("particles", d->emitter);
}

void Demo_Particles2::event(gm::GameMachineEvent evt)
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
