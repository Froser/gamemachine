#include "stdafx.h"
#include "demostration_world.h"
#include <gmcontrolgameobject.h>
#include <gmgl.h>

#include "demo/texture.h"
#include "demo/normalmap.h"
#include "demo/animation.h"
#include "demo/particles1.h"
#include "demo/particles2.h"
#include "demo/effects.h"

void loadDemostrations(DemostrationWorld* world)
{
	world->addDemo("Hello World: Load a texture", new Demo_Texture());
	world->addDemo("Texture advance: Load texture with normal map", new Demo_NormalMap());
	world->addDemo("Animation: How to rotate an object", new Demo_Animation());
	world->addDemo("Particle1: Create a radius particle emitter.", new Demo_Particles1());
	world->addDemo("Particle2: Create a lerp particle emitter.", new Demo_Particles2());
	world->addDemo("Effects: Use a blur effect.", new Demo_Effects());
	world->init();
}

void defaultLookAtFunc()
{
	gm::GMCamera& camera = GM.getCamera();
	gm::CameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, -1 };
	lookAt.position = { 0, 0, 1 };
	camera.lookAt(lookAt);
}

DemoHandler::DemoHandler(void(*lookAtFunc)())
{
	D(d);
	d->lookAtFunc = lookAtFunc;
}

void DemoHandler::init()
{
	D(d);
	d->inited = true;
	d->lookAtFunc();
}

bool DemoHandler::isInited()
{
	D(d);
	return d->inited;
}

void DemoHandler::onActivate()
{
	D(d);
	GM_ASSERT(d->lookAtFunc);
	d->lookAtFunc();
}

void DemoHandler::onDeactivated()
{
	GM.getGraphicEngine()->removeLights();
	GMSetRenderState(EFFECTS, gm::GMEffects::None);
}

DemostrationWorld::~DemostrationWorld()
{
	D(d);
	for (auto& demo : d->demos)
	{
		GM_ASSERT(demo.second);
		delete demo.second;
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
	gm::GMRect rect = { 10, 10, 600, 600 };
	listbox->setGeometry(rect);
	listbox->setItemMargins(0, 5, 0, 0);
	for (auto& demo : d->demos)
	{
		gm::GMImage2DGameObject* item = listbox->addItem(demo.first);
		item->setHeight(20);
		item->attachEvent(*item, gm::GM_CONTROL_EVENT_ENUM(MouseDown), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			if (d->currentDemo)
				d->currentDemo->onDeactivated();
			demo.second->onActivate();
			setCurrentDemo(demo.second);
		});
	}
	addControl(listbox);
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
		if (!currentDemo->isInited())
			currentDemo->init();
		currentDemo->event(evt);
	}

	switch (evt)
	{
	case gm::GameMachineEvent::FrameStart:
		break;
	case gm::GameMachineEvent::FrameEnd:
		break;
	case gm::GameMachineEvent::Simulate:
		getWorld()->notifyControls();
		break;
	case gm::GameMachineEvent::Render:
		getWorld()->renderScene();
		break;
	case gm::GameMachineEvent::Activate:
	{
		gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();

		if (kbState.keydown('Q') || kbState.keydown(VK_ESCAPE))
			GM.postMessage({ gm::GameMachineMessageType::Quit });

		if (kbState.keydown('B'))
			GM.postMessage({ gm::GameMachineMessageType::Console });

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

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	if (d->world)
		delete d->world;
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
