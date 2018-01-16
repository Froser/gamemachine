#include "stdafx.h"
#include "collision.h"
#include <gmdiscretedynamicsworld.h>
#include <linearmath.h>
#include <gmphysicsshape.h>

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

	gm::GMPhysicsShape* groundShape = nullptr;
	gm::GMPhysicsShapeCreator::createBoxShape(glm::vec3(50, 50, 50), &groundShape);
	rigidGround->setShape(groundShape);

	// add to physics world
	physicsWorld->addRigidObjects(rigidGround);

	// add to graphic world
	d->demoWorld->addObject(L"ground", d->ground);
	/*
	// create box
	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
		gm::GMRigidPhysicsObject* box = new gm::GMRigidPhysicsObject();
		

		btBoxShape* colShape = createBoxShape(btVector3(.1, .1, .1));


		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		m_collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);


		for (int k = 0; k < ARRAY_SIZE_Y; k++)
		{
			for (int i = 0; i < ARRAY_SIZE_X; i++)
			{
				for (int j = 0; j < ARRAY_SIZE_Z; j++)
				{
					startTransform.setOrigin(btVector3(
						btScalar(0.2*i),
						btScalar(2 + .2*k),
						btScalar(0.2*j)));


					createRigidBody(mass, startTransform, colShape);


				}
			}
		}
	}
	*/
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
		d->demoWorld->renderScene();
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

void Demo_Collision::setLookAt()
{
	gm::GMCamera& camera = GM.getCamera();
	camera.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = glm::normalize(glm::vec3(-50, 30, 40));
	camera.lookAt(lookAt);
}

void Demo_Collision::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		gm::GMLight light(gm::GMLightType::AMBIENT);
		gm::GMfloat color[] = { .7f, .7f, .7f };
		light.setLightColor(color);
		GM.getGraphicEngine()->addLight(light);
	}
}