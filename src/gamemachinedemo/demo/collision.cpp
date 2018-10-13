#include "stdafx.h"
#include "collision.h"
#include <gmdiscretedynamicsworld.h>
#include <linearmath.h>
#include <gmphysicsshape.h>
#include <gmconstraint.h>
#include <gmgraphicengine.h>
#include <gmmodelreader.h>

namespace
{
	constexpr gm::GMint32 ARRAY_SIZE_X = 5;
	constexpr gm::GMint32 ARRAY_SIZE_Y = 5;
	constexpr gm::GMint32 ARRAY_SIZE_Z = 5;

	static GMVec3 s_colors[4] =
	{
		GMVec3(60.f / 256.f ,186.f / 256.f, 84.f / 256.f),
		GMVec3(244.f / 256.f ,194.f / 256.f, 13.f / 256.f),
		GMVec3(219.f / 256.f ,50.f / 256.f, 54.f / 256.f),
		GMVec3(72.f / 256.f ,133.f / 256.f, 237.f / 256.f),
	};
}

void Demo_Collision::createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset)
{
	gm::GMPhysicsShapeHelper::createCubeShape(GMVec3(.1f, .1f, .1f), asset);
}

void Demo_Collision::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	gm::GMDiscreteDynamicsWorld* physicsWorld = d->discreteWorld = new gm::GMDiscreteDynamicsWorld(getDemoWorldReference().get());
	d->ground = new gm::GMGameObject();
	d->ground->setTranslation(Translate(GMVec3(0, -50, 0)));

	gm::GMRigidPhysicsObject* rigidGround = new gm::GMRigidPhysicsObject();
	d->ground->setPhysicsObject(rigidGround);
	rigidGround->setMass(.0f); //static object

	gm::GMPhysicsShapeAsset groundShape;
	gm::GMPhysicsShapeHelper::createCubeShape(GMVec3(50, 50, 50), groundShape);
	rigidGround->setShape(getDemoWorldReference()->getAssets().addAsset(groundShape));

	gm::GMModel* groundShapeModel = nullptr;
	gm::GMPhysicsShapeHelper::createModelFromShape(groundShape.getPhysicsShape(), &groundShapeModel);
	GM_ASSERT(groundShapeModel);

	groundShapeModel->getShader().getMaterial().ka = GMVec3(.8125f / .7f, .644f / .7f, .043f / .7f);
	groundShapeModel->getShader().getMaterial().kd = GMVec3(.1f);
	groundShapeModel->getShader().getMaterial().ks = GMVec3(.4f);
	groundShapeModel->getShader().getMaterial().shininess = 9;
	d->ground->setAsset(getDemoWorldReference()->getAssets().addAsset(gm::GMAsset(gm::GMAssetType::Model, groundShapeModel)));

	// add to physics world
	physicsWorld->addRigidObject(rigidGround);

	// add to graphic world
	asDemoGameWorld(getDemoWorldReference())->addObject(L"ground", d->ground);

	// create box
	createBoxes();

	createDefaultWidget();
}

void Demo_Collision::createBoxes()
{
	D(d);
	gm::GMPhysicsShapeAsset boxShape;
	createPhysicsShapeAsset(boxShape);
	gm::GMAsset boxAsset = getDemoWorldReference()->getAssets().addAsset(boxShape);

	gm::GMint32 idx = 0;
	for (gm::GMint32 k = 0; k < ARRAY_SIZE_Y; k++)
	{
		for (gm::GMint32 i = 0; i < ARRAY_SIZE_X; i++)
		{
			for (gm::GMint32 j = 0; j < ARRAY_SIZE_Z; j++, idx++)
			{
				gm::GMRigidPhysicsObject* rigidBoxObj = new gm::GMRigidPhysicsObject();
				if (idx == 0)
					d->firstPhyObj = rigidBoxObj; //Record one object

				rigidBoxObj->setMass(1.f);

				gm::GMGameObject* box = new gm::GMGameObject();
				box->setTranslation(Translate(GMVec3(.2f * i, 2 + .2f*k, .2f * j)));
				box->setPhysicsObject(rigidBoxObj);
				rigidBoxObj->setShape(boxAsset);

				gm::GMModel* boxShapeModel = nullptr;
				gm::GMPhysicsShapeHelper::createModelFromShape(boxShape.getPhysicsShape(), &boxShapeModel);
				GM_ASSERT(boxShapeModel);
				// Set color
				boxShapeModel->getShader().getMaterial().ka = s_colors[idx % GM_array_size(s_colors)];
				boxShapeModel->getShader().getMaterial().kd = GMVec3(.1f);
				boxShapeModel->getShader().getMaterial().ks = GMVec3(.4f);
				boxShapeModel->getShader().getMaterial().shininess = 99;

				box->setAsset(getDemoWorldReference()->getAssets().addAsset(gm::GMAsset(gm::GMAssetType::Model, boxShapeModel)));

				d->discreteWorld->addRigidObject(rigidBoxObj);
				asDemoGameWorld(getDemoWorldReference())->addObject(gm::GMString(idx), box);
			}
		}
	}
}

void Demo_Collision::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	Base::event(evt);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::FrameStart:
		break;
	case gm::GameMachineHandlerEvent::FrameEnd:
		break;
	case gm::GameMachineHandlerEvent::Update:
	{
		getDemoWorldReference()->getPhysicsWorld()->update(GM.getRunningStates().lastFrameElpased, nullptr);
		break;
	}
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		onWindowActivate();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Collision::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = Normalize(GMVec3(.5f, -.3f, 1));
	lookAt.position = GMVec3(-1, 2, -3);
	camera.lookAt(lookAt);
}

void Demo_Collision::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();

		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Ambient, &light);
			GM_ASSERT(light);
			gm::GMfloat colorA[] = { .7f, .7f, .7f };
			light->setLightColor(colorA);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}

		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
			GM_ASSERT(light);
			gm::GMfloat colorD[] = { .7f, .7f, .7f };
			light->setLightColor(colorD);

			gm::GMfloat lightPos[] = { -3.f, 3.f, -3.f };
			light->setLightPosition(lightPos);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}

		{
			gm::GMShadowSourceDesc desc;
			desc.position = GMVec4(-3.f, 3.f, -3.f, 1);
			desc.type = gm::GMShadowSourceDesc::DirectShadow;
			desc.camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
			desc.biasMax = desc.biasMin = 0.0005f;
			desc.width = windowStates.renderRect.width * 2;
			desc.height = windowStates.renderRect.height * 2;

			gm::GMCameraLookAt lookAt;
			desc.camera.lookAt(gm::GMCameraLookAt::makeLookAt(desc.position, GMVec3(0, 0, 0)));

			getDemonstrationWorld()->getContext()->getEngine()->setShadowSource(desc);
		}
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
	gm::IInput* input = getDemonstrationWorld()->getMainWindow()->getInputMananger();
	auto ms = input->getMouseState().mouseState();
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();

	if (ms.downButton & gm::GMMouseButton_Left)
	{
		GMVec3 rayFrom = camera.getLookAt().position;
		GMVec3 rayTo = camera.getRayToWorld(getDemonstrationWorld()->getContext()->getWindow()->getRenderRect(), ms.posX, ms.posY);
		gm::GMPhysicsRayTestResult rayTestResult = d->discreteWorld->rayTest(rayFrom, rayTo);

		if (rayTestResult.hit && !(rayTestResult.hitObject->isStaticObject() || rayTestResult.hitObject->isKinematicObject()))
		{
			if (d->lastSelect)
				d->lastSelect->getGameObject()->getModel()->getShader().getMaterial().ka = d->lastColor;

			d->lastSelect = rayTestResult.hitObject;
			GMVec3& ka = rayTestResult.hitObject->getGameObject()->getModel()->getShader().getMaterial().ka;
			d->lastColor = ka;
			ka += .3f;

			// pick one
			pickUp(rayTestResult);
		}
	}
	else if (ms.upButton & gm::GMMouseButton_Left)
	{
		removePicked();
	}
	else if (ms.moving && isActivating())
	{
		GMVec3 rayFrom = camera.getLookAt().position;
		GMVec3 rayTo = camera.getRayToWorld(getDemonstrationWorld()->getContext()->getWindow()->getRenderRect(), ms.posX, ms.posY);
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
	GMVec4 localPivot = GMVec4(rayTestResult.hitPointWorld, 1.f) * body->getCenterOfMassTransformInversed();
	GMFloat4 f4_localPivot;
	localPivot.loadFloat4(f4_localPivot);
	gm::GMPoint2PointConstraint* p2pc = new gm::GMPoint2PointConstraint(body, GMVec3(f4_localPivot[0], f4_localPivot[1], f4_localPivot[2]));
	d->discreteWorld->addConstraint(p2pc, true);
	d->pickedConstraint = p2pc;
	auto setting = p2pc->getConstraintSetting();
	setting.impulseClamp = 30.f;
	setting.tau = .001f;
	p2pc->setConstraintSetting(setting);
	d->oldPickingPos = rayTestResult.rayToWorld;
	d->hitPos = rayTestResult.hitPointWorld;
	d->oldPickingDist = Length(rayTestResult.hitPointWorld - rayTestResult.rayFromWorld);
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
		GMVec3 dir = Normalize(rayTestResult.rayToWorld - rayTestResult.rayFromWorld) * d->oldPickingDist;
		GMVec3 newPivotB = rayTestResult.rayFromWorld + dir;
		d->pickedConstraint->setPivotB(newPivotB);
	}
}

//////////////////////////////////////////////////////////////////////////
void Demo_Collision_Cone::createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset)
{
	gm::GMPhysicsShapeHelper::createConeShape(.1f, .1f, asset);
}

void Demo_Collision_Cylinder::createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset)
{
	gm::GMPhysicsShapeHelper::createCylinderShape(GMVec3(.1f, .1f, .1f), asset);
}

void Demo_Collision_Sphere::createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset)
{
	gm::GMPhysicsShapeHelper::createSphereShape(.1f, asset);
}


//////////////////////////////////////////////////////////////////////////
static GMVec3 s_modelScaling(.02f, .02f, .02f);

void Demo_Collision_Model::createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset)
{
}

void Demo_Collision_Model::createBoxes()
{
	D(d);
	gm::GMPhysicsShapeAsset modelShape;
	gm::GMGamePackage& pk = *GM.getGamePackageManager();
	gm::GMModelLoadSettings loadSettings(
		"teddy/teddy.obj",
		"teddy",
		getDemonstrationWorld()->getContext()
	);

	gm::GMAsset model;
	bool b = gm::GMModelReader::load(loadSettings, model);
	GM_ASSERT(b);
	gm::GMModelAsset m = model.getModels()->getModels().front();
	for (auto& mesh : m.getModel()->getMeshes())
	{
		// teddy.obj是没有法线数据的，需要计算
		mesh->calculateNormals(m.getModel()->getPrimitiveTopologyMode(), m.getModel()->getShader().getFrontFace());
	}

	gm::GMPhysicsShapeHelper::createConvexShapeFromTriangleModel(model, modelShape, false, s_modelScaling);
	gm::GMAsset teddyAsset = getDemoWorldReference()->getAssets().addAsset(modelShape);

	gm::GMint32 idx = 0;
	for (gm::GMint32 k = 0; k < 2; k++)
	{
		for (gm::GMint32 i = 0; i < 2; i++)
		{
			for (gm::GMint32 j = 0; j < 2; j++, idx++)
			{
				gm::GMRigidPhysicsObject* rigidBoxObj = new gm::GMRigidPhysicsObject();
				if (idx == 0)
					d->firstPhyObj = rigidBoxObj;

				rigidBoxObj->setMass(1.f);

				gm::GMGameObject* modelObject = new gm::GMGameObject();
				modelObject->setTranslation(Translate(GMVec3(.6f * i, 2 + .6f*k, .6f * j)));
				modelObject->setScaling(Scale(s_modelScaling));
				modelObject->setPhysicsObject(rigidBoxObj);
				rigidBoxObj->setShape(teddyAsset);

				gm::GMModelAsset m = model.getModels()->getModels().front();
				gm::GMModel* duplicateModel = new gm::GMModel(m);
				// Set color
				duplicateModel->getShader().getMaterial().ka = s_colors[idx % GM_array_size(s_colors)];
				duplicateModel->getShader().getMaterial().kd = GMVec3(.1f);
				duplicateModel->getShader().getMaterial().ks = GMVec3(.4f);
				duplicateModel->getShader().getMaterial().shininess = 99;

				modelObject->setAsset(gm::GMAsset(gm::GMAssetType::Model, duplicateModel));
				d->discreteWorld->addRigidObject(rigidBoxObj);
				asDemoGameWorld(getDemoWorldReference())->addObject(gm::GMString(idx), modelObject);
			}
		}
	}
}
