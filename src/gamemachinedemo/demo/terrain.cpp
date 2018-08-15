#include "stdafx.h"
#include "terrain.h"

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

	gm::GMPrimitiveCreator::createTerrain(*imgMap, -512, -512, 1024, 1024, 30.f, 100, 100, d->terrainModel);
	d->terrain = new gm::GMGameObject(d->terrainModel);

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
			gm::GMfloat colorA[] = { .7f, .7f, .7f };
			light->setLightColor(colorA);
			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
		}
// 
// 		{
// 			gm::ILight* light = nullptr;
// 			GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
// 			GM_ASSERT(light);
// 			gm::GMfloat colorD[] = { .7f, .7f, .7f };
// 			light->setLightColor(colorD);
// 
// 			gm::GMfloat lightPos[] = { -3.f, 3.f, -3.f };
// 			light->setLightPosition(lightPos);
// 			getDemonstrationWorld()->getContext()->getEngine()->addLight(light);
// 		}
	}
}