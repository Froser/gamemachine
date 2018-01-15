#include "stdafx.h"
#include "collision.h"
#include <gmdiscretedynamicsworld.h>
#include <linearmath.h>

Demo_Collision::~Demo_Collision()
{
	D(d);
	gm::GM_delete(d->demoWorld);
}

void Demo_Collision::init()
{
	D(d);
	Base::init();

	d->demoWorld = new gm::GMDemoGameWorld();
	gm::GMDiscreteDynamicsWorld* physicsWorld = new gm::GMDiscreteDynamicsWorld(d->demoWorld);
	d->ground = new gm::GMGameObject();
	d->ground->setTranslation(glm::translate(glm::vec3(0, -50, 0)));

	gm::GMRigidPhysicsObject* rigidGround = new gm::GMRigidPhysicsObject();
	d->ground->setPhysicsObject(rigidGround);
	rigidGround->setMass(.0f); //static object
	rigidGround->initAsBoxShape(glm::vec3(50, 50, 50));

	physicsWorld->addRigidObjects(rigidGround);
}

void Demo_Collision::event(gm::GameMachineEvent evt)
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
		break;
	case gm::GameMachineEvent::Render:
		break;
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