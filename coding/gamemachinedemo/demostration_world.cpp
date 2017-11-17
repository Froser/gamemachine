#include "stdafx.h"
#include "demostration_world.h"
#include <gmcontrolgameobject.h>
#include <gmgl.h>

#include "demo/texture.h"
#include "demo/normalmap.h"
#include "demo/rotation.h"
#include "demo/particles1.h"
#include "demo/particles2.h"
#include "demo/effects.h"
#include "demo/quake3_bsp.h"
#include "demo/border.h"
#include "demo/sound.h"
#include "demo/literature.h"
#include "demo/button.h"

void loadDemostrations(DemostrationWorld* world)
{
	world->addDemo("Hello World: Load a texture", new Demo_Texture(world));
	world->addDemo("Texture advance: Load texture with normal map", new Demo_NormalMap(world));
	world->addDemo("Rotation: How to rotate an object", new Demo_Rotation(world));
	world->addDemo("Particle1: Create a radius particle emitter.", new Demo_Particles1(world));
	world->addDemo("Particle2: Create a lerp particle emitter.", new Demo_Particles2(world));
	world->addDemo("Effects: Use a blur effect.", new Demo_Effects(world));
	world->addDemo("BSP: Demostrate a Quake3 scene.", new Demo_Quake3_BSP(world));
	world->addDemo("Border: Demostrate a border.", new Demo_Border(world));
	world->addDemo("Sound: Demostrate playing music.", new Demo_Sound(world));
	world->addDemo("Literature: Demostrate render literatures via GMTypoEngine.", new Demo_Literature(world));
	world->addDemo("Button: Demostrate how to create a button.", new Demo_Button(world));
	world->init();
}

DemoHandler::DemoHandler(DemostrationWorld* demostrationWorld)
{
	D(d);
	d->demostrationWorld = demostrationWorld;
}

void DemoHandler::init()
{
	D(d);
	d->inited = true;
	setLookAt();
}

bool DemoHandler::isInited()
{
	D(d);
	return d->inited;
}

void DemoHandler::onActivate()
{
	D(d);
	setLookAt();
	setDefaultLights();
}

void DemoHandler::onDeactivate()
{
	GM.getGraphicEngine()->removeLights();
	GMSetRenderState(EFFECTS, gm::GMEffects::None);
}

void DemoHandler::event(gm::GameMachineEvent evt)
{
	switch (evt)
	{
	case gm::GameMachineEvent::Activate:
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();

		if (kbState.keyTriggered(VK_ESCAPE))
			backToEntrance();

		if (kbState.keyTriggered('B'))
			GM.postMessage({ gm::GameMachineMessageType::Console });

		if (kbState.keyTriggered('L'))
			GMSetDebugState(POLYGON_LINE_MODE, !GMGetDebugState(POLYGON_LINE_MODE));

		if (kbState.keyTriggered('I'))
			GMSetDebugState(RUN_PROFILE, !GMGetDebugState(RUN_PROFILE));
		break;
	}
}

void DemoHandler::setLookAt()
{
	gm::GMCamera& camera = GM.getCamera();
	gm::CameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, -1 };
	lookAt.position = { 0, 0, 1 };
	camera.lookAt(lookAt);
}

void DemoHandler::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		gm::GMLight light(gm::GMLightType::SPECULAR);
		gm::GMfloat lightPos[] = { 0, 0, .2f };
		light.setLightPosition(lightPos);
		gm::GMfloat color[] = { .7f, .7f, .7f };
		light.setLightColor(color);
		GM.getGraphicEngine()->addLight(light);
	}
}

void DemoHandler::backToEntrance()
{
	D(d);
	d->demostrationWorld->setCurrentDemo(nullptr);
	onDeactivate();
}

DemostrationWorld::~DemostrationWorld()
{
	D(d);
	for (auto& demo : d->demos)
	{
		GM_ASSERT(demo.second);
		gm::GM_delete(demo.second);
	}
}

void DemostrationWorld::addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo)
{
	D(d);
	GM_ASSERT(demo);
	d->demos.push_back(std::make_pair(name, demo));
}

void DemostrationWorld::init()
{
	D(d);
	gm::GMListbox2DGameObject* listbox = new gm::GMListbox2DGameObject();

	// 读取边框
	gm::GMGamePackage* package = GM.getGamePackageManager();
	gm::GMBuffer buf;
	bool b = package->readFile(gm::GMPackageIndex::Textures, "border.png", &buf);
	GM_ASSERT(b);
	gm::GMImage* img = nullptr;
	gm::GMImageReader::load(buf.buffer, buf.size, &img);
	gm::ITexture* frameTexture = nullptr;
	GM.getFactory()->createTexture(img, &frameTexture);
	GM_ASSERT(frameTexture);
	gm::GMAsset borderAsset = getAssets().insertAsset(gm::GMAssetType::Texture, frameTexture);
	gm::GMRect textureGeo = { 0, 0, 308, 94 }; //截取的纹理位置

	gm::GMRect rect = { 10, 10, 600, 500 };
	listbox->setGeometry(rect);
	listbox->setItemMargins(0, 5, 0, 0);
	for (auto& demo : d->demos)
	{
		gm::GMImage2DGameObject* item = listbox->addItem(demo.first);
		item->setBorder(gm::GMImage2DBorder(
			borderAsset,
			textureGeo,
			img->getWidth(),
			img->getHeight(),
			14,
			14
		));
		item->setHeight(30);
		item->setPaddings(10, 8, 10, 8);
		item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->nextDemo = demo.second;
		});
	}
	addControl(listbox);
	GM_delete(img);
}

void DemostrationWorld::renderScene()
{
	D(d);
	gm::IGraphicEngine* engine = GM.getGraphicEngine();

	engine->beginBlend();
	Base::renderScene();
	auto& controls = getControlsGameObject();
	engine->drawObjects(controls.data(), controls.size());
	engine->endBlend();
}

void DemostrationWorld::switchDemo()
{
	D(d);
	if (d->nextDemo)
	{
		if (!d->nextDemo->isInited())
			d->nextDemo->init();
		d->nextDemo->onActivate();
		setCurrentDemo(d->nextDemo);
		d->nextDemo = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init()
{
	D(d);

	gm::GMGLGraphicEngine* engine = static_cast<gm::GMGLGraphicEngine*> (GM.getGraphicEngine());
	engine->setShaderLoadCallback(this);
	GMSetRenderState(RENDER_MODE, gm::GMStates_RenderOptions::DEFERRED);
	//GMSetRenderState(EFFECTS, GMEffects::Grayscale);
	GMSetRenderState(RESOLUTION_X, 800);
	GMSetRenderState(RESOLUTION_Y, 600);

	gm::GMGamePackage* pk = GM.getGamePackageManager();
#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((gm::GMPath::getCurrentPath() + _L("gm.pk0")));
#endif
	d->world = new DemostrationWorld();
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	inputManager->getMouseState().setMouseEnable(false);

	// 设置一个默认视角
	gm::GMCamera& camera = GM.getCamera();
	gm::CameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, -1 };
	lookAt.position = { 0, 0, 1 };
	camera.lookAt(lookAt);

	loadDemostrations(d->world);
}

void DemostrationEntrance::event(gm::GameMachineEvent evt)
{
	D(d);
	gm::IGraphicEngine* engine = GM.getGraphicEngine();
	engine->newFrame();

	DemoHandler* currentDemo = getWorld()->getCurrentDemo();
	if (currentDemo)
	{
		currentDemo->event(evt);
	}
	else
	{
		switch (evt)
		{
		case gm::GameMachineEvent::FrameStart:
			break;
		case gm::GameMachineEvent::FrameEnd:
			getWorld()->switchDemo();
			break;
		case gm::GameMachineEvent::Simulate:
			break;
		case gm::GameMachineEvent::Render:
			getWorld()->renderScene();
			break;
		case gm::GameMachineEvent::Activate:
		{
			getWorld()->notifyControls();

			gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
			gm::IKeyboardState& kbState = inputManager->getKeyboardState();

			if (kbState.keyTriggered('Q') || kbState.keyTriggered(VK_ESCAPE))
				GM.postMessage({ gm::GameMachineMessageType::Quit });

			if (kbState.keyTriggered('B'))
				GM.postMessage({ gm::GameMachineMessageType::Console });

			if (kbState.keyTriggered('L'))
				GMSetDebugState(POLYGON_LINE_MODE, !GMGetDebugState(POLYGON_LINE_MODE));

			if (kbState.keyTriggered('I'))
				GMSetDebugState(RUN_PROFILE, !GMGetDebugState(RUN_PROFILE));

			break;
		}
		case gm::GameMachineEvent::Deactivate:
			break;
		case gm::GameMachineEvent::Terminate:
			break;
		default:
			break;
		}
	}
}

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	gm::GM_delete(d->world);
}

void DemostrationEntrance::onLoadForwardShader(const gm::GMMeshType type, gm::GMGLShaderProgram& shader)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	switch (type)
	{
	case gm::GMMeshType::Model3D:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model3d.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model3d.frag", &fragBuf, &fragPath);
		break;
	case gm::GMMeshType::Model2D:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model2d.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "model2d.frag", &fragBuf, &fragPath);
		break;
	case gm::GMMeshType::Glyph:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "glyph.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "glyph.frag", &fragBuf, &fragPath);
		break;
	case gm::GMMeshType::Particles:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "particles.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "particles.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}

	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shader.attachShader(shadersInfo[0]);
	shader.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredPassShader(gm::GMGLDeferredRenderState state, gm::GMGLShaderProgram& shaderProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	switch (state)
	{
	case gm::GMGLDeferredRenderState::PassingGeometry:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/geometry_pass.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/geometry_pass.frag", &fragBuf, &fragPath);
		break;
	case gm::GMGLDeferredRenderState::PassingMaterial:
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/material_pass.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/material_pass.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shaderProgram.attachShader(shadersInfo[0]);
	shaderProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredLightPassShader(gm::GMGLShaderProgram& lightPassProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/light_pass.vert", &vertBuf, &vertPath);
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "deferred/light_pass.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadEffectsShader(gm::GMGLShaderProgram& lightPassProgram)
{
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "effects/effects.vert", &vertBuf, &vertPath);
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "effects/effects.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}
