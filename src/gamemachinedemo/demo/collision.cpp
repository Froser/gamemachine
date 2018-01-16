#include "stdafx.h"
#include "collision.h"
#include <gmdiscretedynamicsworld.h>
#include <linearmath.h>
#include <gmphysicsshape.h>
#include <gmbullethelper.h>

namespace
{
	const gm::GMint ARRAY_SIZE_X = 5;
	const gm::GMint ARRAY_SIZE_Y = 5;
	const gm::GMint ARRAY_SIZE_Z = 5;
}

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
	rigidGround->setShape(d->demoWorld->getAssets().insertAsset(gm::GMAssetType::PhysicsShape, groundShape));

	gm::GMAsset groundShapeModel = gm::GMBulletHelper::createModelFromShape(groundShape);
	d->ground->setModel(groundShapeModel);
	d->demoWorld->getAssets().insertAsset(groundShapeModel);

	// add to physics world
	physicsWorld->addRigidObject(rigidGround);

	// add to graphic world
	d->demoWorld->addObject(L"ground", d->ground);

	// create box
	{
		// create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance
		gm::GMPhysicsShape* boxShape = nullptr;
		gm::GMPhysicsShapeCreator::createBoxShape(glm::vec3(.1f, .1f, .1f), &boxShape);
		gm::GMAsset boxAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::PhysicsShape, boxShape);

		// create dynamic objects
		// rigidbody is dynamic if and only if mass is non zero, otherwise static
		for (gm::GMint k = 0; k < ARRAY_SIZE_Y; k++)
		{
			for (gm::GMint i = 0; i < ARRAY_SIZE_X; i++)
			{
				for (gm::GMint j = 0; j < ARRAY_SIZE_Z; j++)
				{
					gm::GMRigidPhysicsObject* rigidBoxObj = new gm::GMRigidPhysicsObject();
					rigidBoxObj->setMass(1.f);

					gm::GMGameObject* box = new gm::GMGameObject();
					box->setTranslation(glm::translate(glm::vec3(.2f * i, 2 + .2f*k, .2f * j)));
					box->setPhysicsObject(rigidBoxObj);

					rigidBoxObj->setShape(boxAsset);
					//TODO rigidBoxObj->createMeshForObject(d->demoWorld);
					physicsWorld->addRigidObject(rigidBoxObj);

					d->demoWorld->addObject(gm::GMString(i), box);
				}
			}
		}
	}
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
	lookAt.lookAt = glm::normalize(glm::vec3(1, 1, 1));
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