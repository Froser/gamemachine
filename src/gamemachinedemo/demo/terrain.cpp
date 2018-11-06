#include "stdafx.h"
#include "terrain.h"

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

void Demo_Terrain::init()
{
	D(d);
	D_BASE(db, DemoHandler);
	Base::init();

	getDemoWorldReference().reset(new gm::GMDemoGameWorld(db->parentDemonstrationWorld->getContext()));
	createDefaultWidget();

	gm::GMBuffer map;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Textures, "map.png", &map);

	gm::GMImage* imgMap = nullptr;
	gm::GMImageReader::load(map.buffer, map.size, &imgMap);

	gm::GMTerrainDescription desc = {
		imgMap->getData().mip[0].data,
		imgMap->getData().channels,
		imgMap->getWidth(),
		imgMap->getHeight(),
		-256.f,
		-256.f,
		512.f,
		512.f,
		80.f,
		100,
		100,
		100,
		100
	};

	gm::GMPrimitiveCreator::createTerrain(desc, d->terrainScene);
	gm::GMModel* terrainModel = d->terrainScene.getScene()->getModels()[0].getModel();
	gm::GMTextureAsset texture = gm::GMToolUtil::createTexture(db->parentDemonstrationWorld->getContext(), L"grass.jpg");
	gm::GMToolUtil::addTextureToShader(terrainModel->getShader(), texture, gm::GMTextureType::Ambient);
	terrainModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapS(gm::GMS_Wrap::Repeat);
	terrainModel->getShader().getTextureList().getTextureSampler(gm::GMTextureType::Ambient).setWrapT(gm::GMS_Wrap::Repeat);
	d->terrain = new gm::GMGameObject(d->terrainScene);

	asDemoGameWorld(getDemoWorldReference())->addObject(L"terrain", d->terrain);
}

void Demo_Terrain::event(gm::GameMachineHandlerEvent evt)
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
		break;
	case gm::GameMachineHandlerEvent::Render:
		getDemoWorldReference()->renderScene();
		break;
	case gm::GameMachineHandlerEvent::Activate:
		handleMouseEvent();
		handleDragging();
		break;
	case gm::GameMachineHandlerEvent::Deactivate:
		break;
	case gm::GameMachineHandlerEvent::Terminate:
		break;
	default:
		break;
	}
}

void Demo_Terrain::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
	camera.setPerspective(Radians(75.f), 1.333f, .1f, 3200);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = Normalize(GMVec3(0, -.5f, 1));
	lookAt.position = GMVec3(0, 100, 0);
	camera.lookAt(lookAt);
}

void Demo_Terrain::setDefaultLights()
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
			gm::GMfloat colorA[] = { .5f, .5f, .5f };
			light->setLightColor(colorA);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}

		{
			gm::ILight* light = nullptr;
			GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
			GM_ASSERT(light);
			gm::GMfloat colorD[] = { .3f, .3f, .3f };
			light->setLightColor(colorD);

			gm::GMfloat lightPos[] = { 100.f, 100.f, 100.f };
			light->setLightPosition(lightPos);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
	}
}

void Demo_Terrain::handleMouseEvent()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	const gm::GMWindowStates& windowStates = getDemonstrationWorld()->getContext()->getWindow()->getWindowStates();
	if (state.wheel.wheeled)
	{
		gm::GMfloat delta = .05f * state.wheel.delta / WHEEL_DELTA;
		GMFloat4 scaling;
		{
			GetScalingFromMatrix(d->terrain->getScaling(), scaling);
			scaling[0] += delta;
			scaling[1] += delta;
			scaling[2] += delta;
			if (scaling[0] > 0 && scaling[1] > 0 && scaling[2] > 0)
				d->terrain->setScaling(Scale(GMVec3(scaling[0], scaling[1], scaling[2])));
		}
	}

	if (d->draggingL)
	{
		gm::GMfloat rotateX = state.posX - d->mouseDownX;
		GMQuat q = Rotate(d->terrain->getRotation(),
			PI * rotateX / windowStates.renderRect.width,
			GMVec3(0, 1, 0));
		d->terrain->setRotation(q);
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
	}
}

void Demo_Terrain::handleDragging()
{
	D(d);
	gm::IMouseState& ms = getDemonstrationWorld()->getMainWindow()->getInputMananger()->getMouseState();
	gm::GMMouseState state = ms.mouseState();
	if (state.downButton & gm::GMMouseButton_Left)
	{
		d->mouseDownX = state.posX;
		d->mouseDownY = state.posY;
		d->draggingL = true;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(true);
	}
	else if (state.upButton & gm::GMMouseButton_Left)
	{
		d->draggingL = false;
		getDemonstrationWorld()->getMainWindow()->setWindowCapture(false);
	}
}