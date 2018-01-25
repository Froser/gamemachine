#include "stdafx.h"
#include "collision.h"
#include <gmdiscretedynamicsworld.h>
#include <linearmath.h>
#include <gmphysicsshape.h>
#include <gmbullethelper.h>
#include <gmconstraint.h>

namespace
{
	const gm::GMint ARRAY_SIZE_X = 5;
	const gm::GMint ARRAY_SIZE_Y = 5;
	const gm::GMint ARRAY_SIZE_Z = 5;

	static glm::vec3 s_colors[4] =
	{
		glm::vec3(60.f / 256.f ,186.f / 256.f, 84.f / 256.f),
		glm::vec3(244.f / 256.f ,194.f / 256.f, 13.f / 256.f),
		glm::vec3(219.f / 256.f ,50.f / 256.f, 54.f / 256.f),
		glm::vec3(72.f / 256.f ,133.f / 256.f, 237.f / 256.f),
	};
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
	gm::GMDiscreteDynamicsWorld* physicsWorld = d->discreteWorld = new gm::GMDiscreteDynamicsWorld(d->demoWorld);
	d->ground = new gm::GMGameObject();
	d->ground->setTranslation(glm::translate(glm::vec3(0, -50, 0)));

	gm::GMRigidPhysicsObject* rigidGround = new gm::GMRigidPhysicsObject();
	d->ground->setPhysicsObject(rigidGround);
	rigidGround->setMass(.0f); //static object

	gm::GMPhysicsShape* groundShape = nullptr;
	gm::GMPhysicsShapeCreator::createBoxShape(glm::vec3(50, 50, 50), &groundShape);
	rigidGround->setShape(d->demoWorld->getAssets().insertAsset(gm::GMAssetType::PhysicsShape, groundShape));

	gm::GMModel* groundShapeModel = nullptr;
	gm::GMBulletHelper::createModelFromShape(groundShape, &groundShapeModel);
	GM_ASSERT(groundShapeModel);

	auto& components = groundShapeModel->getMesh()->getComponents();
	for (auto& component : components)
	{
		component->getShader().getMaterial().ka = glm::vec3(.8125f / .7f, .644f / .7f, .043f / .7f);
		component->getShader().getMaterial().kd = glm::vec3(.1f);
		component->getShader().getMaterial().ks = glm::vec3(.4f);
		component->getShader().getMaterial().shininess = 9;
	}
	d->ground->setModel(d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, groundShapeModel));

	// add to physics world
	physicsWorld->addRigidObject(rigidGround);

	// add to graphic world
	d->demoWorld->addObject(L"ground", d->ground);

	// create box
	{
		gm::GMPhysicsShape* boxShape = nullptr;
		gm::GMPhysicsShapeCreator::createBoxShape(glm::vec3(.1f, .1f, .1f), &boxShape);
		gm::GMAsset boxAsset = d->demoWorld->getAssets().insertAsset(gm::GMAssetType::PhysicsShape, boxShape);

		gm::GMint idx = 0;
		for (gm::GMint k = 0; k < ARRAY_SIZE_Y; k++)
		{
			for (gm::GMint i = 0; i < ARRAY_SIZE_X; i++)
			{
				for (gm::GMint j = 0; j < ARRAY_SIZE_Z; j++, idx++)
				{
					gm::GMRigidPhysicsObject* rigidBoxObj = new gm::GMRigidPhysicsObject();
					if (idx == 0)
						d->firstPhyObj = rigidBoxObj; //Record one object

					rigidBoxObj->setMass(1.f);

					gm::GMGameObject* box = new gm::GMGameObject();
					box->setTranslation(glm::translate(glm::vec3(.2f * i, 2 + .2f*k, .2f * j)));
					box->setPhysicsObject(rigidBoxObj);
					rigidBoxObj->setShape(boxAsset);

					gm::GMModel* boxShapeModel = nullptr;
					gm::GMBulletHelper::createModelFromShape(boxShape, &boxShapeModel);
					GM_ASSERT(boxShapeModel);
					// Set color
					auto& components = boxShapeModel->getMesh()->getComponents();
					for (auto& component : components)
					{
						component->getShader().getMaterial().ka = s_colors[idx % GM_array_size(s_colors)];
						component->getShader().getMaterial().kd = glm::vec3(.1f);
						component->getShader().getMaterial().ks = glm::vec3(.4f);
						component->getShader().getMaterial().shininess = 99;
					}

					box->setModel(d->demoWorld->getAssets().insertAsset(gm::GMAssetType::Model, boxShapeModel));

					physicsWorld->addRigidObject(rigidBoxObj);
					d->demoWorld->addObject(gm::GMString(idx), box);
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
	{
		d->demoWorld->getPhysicsWorld()->simulate(nullptr);
		const gm::GMMotionStates& states = d->firstPhyObj->getMotionStates(); // Get current motion states
		break;
	}
	case gm::GameMachineEvent::Render:
		d->demoWorld->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
		onWindowActivate();
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
	lookAt.lookAt = glm::normalize(glm::vec3(.5f, -.3f, 1));
	lookAt.position = glm::vec3(-1, 2, -3);
	camera.lookAt(lookAt);
}

void Demo_Collision::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		gm::GMLight a(gm::GMLightType::AMBIENT);
		gm::GMfloat colorA[] = { .7f, .7f, .7f };
		a.setLightColor(colorA);
		GM.getGraphicEngine()->addLight(a);

		gm::GMLight d(gm::GMLightType::SPECULAR);
		gm::GMfloat colorD[] = { .7f, .7f, .7f };
		d.setLightColor(colorD);
		d.setLightPosition(glm::value_ptr(glm::vec3(-1.f, .5f, -3.f)));
		GM.getGraphicEngine()->addLight(d);
	}
}

void Demo_Collision::onDeactivate()
{
	removePicked();
	Base::onDeactivate();
}

void Demo_Collision::onWindowActivate()
{
	D(d);
	gm::IInput* input = GM.getMainWindow()->getInputMananger();
	auto& ms = input->getMouseState().mouseState();
	gm::GMCamera& camera = GM.getCamera();

	if (ms.downButton & GMMouseButton_Left)
	{
		glm::vec3 rayFrom = camera.getLookAt().position;
		glm::vec3 rayTo = camera.getRayToWorld(ms.posX, ms.posY);
		gm::GMPhysicsRayTestResult rayTestResult = d->discreteWorld->rayTest(rayFrom, rayTo);

		if (rayTestResult.hit && !(rayTestResult.hitObject->isStaticObject() || rayTestResult.hitObject->isKinematicObject()))
		{
			if (d->lastSelect)
				d->lastSelect->getGameObject()->getModel()->getMesh()->getComponents()[0]->getShader().getMaterial().ka = d->lastColor;

			d->lastSelect = rayTestResult.hitObject;
			glm::vec3& ka = rayTestResult.hitObject->getGameObject()->getModel()->getMesh()->getComponents()[0]->getShader().getMaterial().ka;
			d->lastColor = ka;
			ka += .3f;

			// pick one
			pickUp(rayTestResult);
		}
	}
	else if (ms.upButton & GMMouseButton_Left)
	{
		removePicked();
	}
	else if (ms.moving && isActivating())
	{
		glm::vec3 rayFrom = camera.getLookAt().position;
		glm::vec3 rayTo = camera.getRayToWorld(ms.posX, ms.posY);
		gm::GMPhysicsRayTestResult rayTestResult = d->discreteWorld->rayTest(rayFrom, rayTo);
		movePicked(rayTestResult);
	}
}

void Demo_Collision::pickUp(const gm::GMPhysicsRayTestResult& rayTestResult)
{
	D(d);
	gm::GMRigidPhysicsObject* body = d->pickedBody = rayTestResult.hitObject;
	d->pickedActivationState = body->getActivationState();
	body->setActivationState(gm::GMPhysicsActivationState::DisableDeactivation);
	glm::vec4 localPivot = body->getCenterOfMassTransformInversed() * glm::vec4(rayTestResult.hitPointWorld, 1.f);
	gm::GMPoint2PointConstraint* p2pc = new gm::GMPoint2PointConstraint(body, glm::vec3(localPivot[0], localPivot[1], localPivot[2]));
	d->discreteWorld->addConstraint(p2pc, true);
	d->pickedConstraint = p2pc;
	auto settings = p2pc->getConstraintSetting();
	settings.impulseClamp = 30.f;
	settings.tau = .001f;
	p2pc->setConstraintSetting(settings);
	d->oldPickingPos = rayTestResult.rayToWorld;
	d->hitPos = rayTestResult.hitPointWorld;
	d->oldPickingDist = glm::fastLength(rayTestResult.hitPointWorld - rayTestResult.rayFromWorld);
}

void Demo_Collision::removePicked()
{
	D(d);
	if (d->pickedBody)
	{
		GM_ASSERT(d->pickedConstraint);

		d->pickedBody->setActivationState(d->pickedActivationState, true);
		d->pickedBody->activate();
		d->discreteWorld->removeConstraint(d->pickedConstraint);
		GM_delete(d->pickedConstraint);
		d->pickedBody = nullptr;
	}
}

void Demo_Collision::movePicked(const gm::GMPhysicsRayTestResult& rayTestResult)
{
	D(d);
	if (d->pickedBody && d->pickedConstraint)
	{
		glm::vec3 dir = glm::normalize(rayTestResult.rayToWorld - rayTestResult.rayFromWorld) * d->oldPickingDist;
		glm::vec3 newPivotB = rayTestResult.rayFromWorld + dir;
		d->pickedConstraint->setPivotB(newPivotB);
	}
}