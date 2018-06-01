#include "stdafx.h"
#include "demonstration_world.h"
#include <gmwidget.h>
#include <gmgl.h>
#include <gmgraphicengine.h>

#include "demo/texture.h"
//#include "demo/normalmap.h"
//#include "demo/effects.h"
//#include "demo/quake3_bsp.h"
//#include "demo/sound.h"
//#include "demo/literature.h"
//#include "demo/model.h"
//#include "demo/collision.h"
//#include "demo/specularmap.h"
//#include "demo/pbr.h"
//#include "demo/phong_pbr.h"

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif

extern gm::GMRenderEnvironment GetRenderEnv();

namespace
{
	void loadDemostrations(DemonstrationWorld* world)
	{
		world->addDemo("Hello World: Load a texture", new Demo_Texture(world));
		//world->addDemo("Hello World: Load a texture with indices buffer", new Demo_Texture_Index(world));
		//world->addDemo("Texture advance: Load texture with normal map", new Demo_NormalMap(world));
		//world->addDemo("Effects: Use a grayscale filter.", new Demo_Effects(world));
		//world->addDemo("BSP: Demonstrate a Quake3 scene.", new Demo_Quake3_BSP(world));
		//world->addDemo("Sound: Demonstrate playing music.", new Demo_Sound(world));
		//world->addDemo("Literature: Demonstrate render literatures via GMTypoEngine.", new Demo_Literature(world));
		//world->addDemo("Model: Load a model. Adjust model by dragging or wheeling.", new Demo_Model(world));
		//world->addDemo("Physics: Demonstrate collision objects.", new Demo_Collision(world));
		//world->addDemo("SpecularMap: Demonstrate a cube with specular map.", new Demo_SpecularMap(world));
		//world->addDemo("PBR: Demonstrate a scene with PBR.", new Demo_PBR(world));
		//world->addDemo("PBR: Demonstrate a scene with both Phong and PBR.", new Demo_Phong_PBR(world));
		world->init();
	}
}

DemoHandler::DemoHandler(DemonstrationWorld* parentDemonstrationWorld)
{
	D(d);
	d->parentDemonstrationWorld = parentDemonstrationWorld;
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
	d->engine = parentDemonstrationWorld->getContext()->engine;
}

DemoHandler::~DemoHandler()
{
	D(d);
	GM_delete(getDemoWorldReference());
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
	d->activating = true;
}

void DemoHandler::onDeactivate()
{
	D(d);
	d->parentDemonstrationWorld->resetProjectionAndEye();
	d->engine->removeLights();
	d->renderConfig.set(gm::GMRenderConfigs::FilterMode, gm::GMFilterMode::None);
	d->activating = false;

	gm::GMShadowSourceDesc noShadow;
	noShadow.type = gm::GMShadowSourceDesc::NoShadow;
	d->engine->setShadowSource(noShadow);

	d->parentDemonstrationWorld->getMainWidget()->setVisible(true);
}

void DemoHandler::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Activate:
		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();

		if (kbState.keyTriggered(VK_ESCAPE))
			backToEntrance();

		if (kbState.keyTriggered('L'))
			d->debugConfig.set(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool, !d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool).toBool());

		if (kbState.keyTriggered('I'))
			d->debugConfig.set(gm::GMDebugConfigs::RunProfile_Bool, !d->debugConfig.get(gm::GMDebugConfigs::RunProfile_Bool).toBool());

		if (kbState.keyTriggered('Z'))
			d->renderConfig.set(gm::GMRenderConfigs::GammaCorrection_Bool, !d->renderConfig.get(gm::GMRenderConfigs::GammaCorrection_Bool).toBool());

		if (kbState.keyTriggered('T'))
		{
			if (getDemoWorldReference()->getRenderPreference() == gm::GMRenderPreference::PreferDeferredRendering)
				getDemoWorldReference()->setRenderPreference(gm::GMRenderPreference::PreferForwardRendering);
			else
				getDemoWorldReference()->setRenderPreference(gm::GMRenderPreference::PreferDeferredRendering);
		}
		break;
	}
}

void DemoHandler::setLookAt()
{
	gm::GMCamera& camera = GM.getCamera();
	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, 1 };
	lookAt.position = { 0, 0, -1 };
	camera.lookAt(lookAt);
}

void DemoHandler::setDefaultLights()
{
	// 所有Demo的默认灯光属性
	D(d);
	if (isInited())
	{
		gm::ILight* light = nullptr;
		GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
		GM_ASSERT(light);
		gm::GMfloat lightPos[] = { 0, 0, -.2f };
		light->setLightPosition(lightPos);
		gm::GMfloat color[] = { .7f, .7f, .7f };
		light->setLightColor(color);
		d->engine->addLight(light);
	}
}

void DemoHandler::backToEntrance()
{
	D(d);
	d->parentDemonstrationWorld->setCurrentDemo(nullptr);
	onDeactivate();
}

bool DemoHandler::isActivating()
{
	D(d);
	return d->activating;
}

void DemoHandler::switchNormal()
{
	D(d);
	d->debugConfig.set(
		gm::GMDebugConfigs::DrawPolygonNormalMode,
		(d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonNormalMode).toInt() + 1) % gm::GMDrawPolygonNormalMode::EndOfEnum
	);
}

DemonstrationWorld::DemonstrationWorld(const gm::GMContext* context, gm::IWindow* window)
	: Base(context)
{
	D(d);
	d->mainWindow = window;
}

DemonstrationWorld::~DemonstrationWorld()
{
	D(d);
	for (auto& demo : d->demos)
	{
		GM_ASSERT(demo.second);
		gm::GM_delete(demo.second);
	}

	GM_delete(d->mainWidget);
}

void DemonstrationWorld::addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo)
{
	D(d);
	GM_ASSERT(demo);
	d->demos.push_back(std::make_pair(name, demo));
}

void DemonstrationWorld::init()
{
	D(d);
	gm::GMGamePackage* package = GM.getGamePackageManager();

	// 创建画布
	auto manager = new gm::GMWidgetResourceManager(getContext());
	gm::ITexture* texture = nullptr;
	gm::GMint width, height;
	gm::GMToolUtil::createTexture(getContext(), "skin.png", &texture, &width, &height);
	manager->addTexture(texture, width, height);

	d->mainWidget = new gm::GMWidget(manager);
	{
		gm::GMRect rc = { 0, 0, 136, 54 };
		d->mainWidget->addArea(gm::GMCanvasControlArea::ButtonArea, rc);
	}
	{
		gm::GMRect rc = { 136, 0, 116, 54 };
		d->mainWidget->addArea(gm::GMCanvasControlArea::ButtonFillArea, rc);
	}

	manager->registerWidget(d->mainWidget);
	d->mainWidget->init();
	d->mainWidget->setKeyboardInput(true);

	gm::GMint Y = 10, marginY = 10;
	for (auto& demo : d->demos)
	{
		gm::GMControlButton* button = nullptr;
		d->mainWidget->addButton(
			0,
			demo.first,
			10,
			Y,
			600,
			30,
			false,
			&button
		);
		Y += 30 + marginY;
		GM_ASSERT(button);
		button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->nextDemo = demo.second;
			d->mainWidget->setVisible(false);
		});
	}

	d->mainWidget->addStatic(-1, "Hello world", 600, 400, 100, 100, false, nullptr);
	d->mainWidget->addStatic(-1, "GameMachine", 700, 450, 100, 100, false, nullptr);
	d->mainWindow->addWidget(d->mainWidget);
}

void DemonstrationWorld::switchDemo()
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

void DemonstrationWorld::resetProjectionAndEye()
{
	// 设置一个默认视角
	gm::GMCamera& camera = GM.getCamera();
	camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, 1 };
	lookAt.position = { 0, 0, -1 };
	camera.lookAt(lookAt);
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init(const gm::GMContext* context)
{
	D(d);
	auto& rc = context->window->getRenderRect();

	gm::GMGamePackage* pk = GM.getGamePackageManager();

#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#endif

	context->engine->setShaderLoadCallback(this);
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
	d->world = new DemonstrationWorld(context, d->mainWindow);
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = d->mainWindow->getInputMananger();
	getWorld()->resetProjectionAndEye();
	loadDemostrations(d->world);
}

void DemostrationEntrance::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	if (evt == gm::GameMachineHandlerEvent::Render)
	{
		gm::IGraphicEngine* engine = getWorld()->getContext()->engine;
		engine->getDefaultFramebuffers()->clear();
		engine->getDefaultFramebuffers()->bind();
	}

	DemoHandler* currentDemo = getWorld()->getCurrentDemo();
	if (currentDemo)
	{
		currentDemo->event(evt);
	}
	else
	{
		switch (evt)
		{
		case gm::GameMachineHandlerEvent::FrameStart:
			break;
		case gm::GameMachineHandlerEvent::FrameEnd:
			getWorld()->switchDemo();
			break;
		case gm::GameMachineHandlerEvent::Simulate:
			break;
		case gm::GameMachineHandlerEvent::Render:
			getWorld()->renderScene();
			break;
		case gm::GameMachineHandlerEvent::Activate:
		{
			gm::IInput* inputManager = d->mainWindow->getInputMananger();
			gm::IKeyboardState& kbState = inputManager->getKeyboardState();

			if (kbState.keyTriggered('Q') || kbState.keyTriggered(VK_ESCAPE))
				GM.postMessage({ gm::GameMachineMessageType::QuitGameMachine });

			if (kbState.keyTriggered('L'))
				d->debugConfig.set(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool, !d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool).toBool());

			if (kbState.keyTriggered('I'))
				d->debugConfig.set(gm::GMDebugConfigs::RunProfile_Bool, !d->debugConfig.get(gm::GMDebugConfigs::RunProfile_Bool).toBool());

			break;
		}
		case gm::GameMachineHandlerEvent::Deactivate:
			break;
		case gm::GameMachineHandlerEvent::Terminate:
			break;
		default:
			break;
		}
	}
}

DemostrationEntrance::DemostrationEntrance(gm::IWindow* window)
{
	D(d);
	d->mainWindow = window;
}

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	gm::GM_delete(d->world);
}

void DemostrationEntrance::onLoadShaders(const gm::GMContext* context)
{
	D(d);
	auto& env = GM.getGameMachineRunningStates().renderEnvironment;

	if (env == gm::GMRenderEnvironment::OpenGL)
	{
		bool b;
		gm::GMGLShaderProgram* effectsShaderProgram = new gm::GMGLShaderProgram(context);
		initLoadEffectsShader(effectsShaderProgram);
		b = context->engine->setInterface(gm::GameMachineInterfaceID::GLEffectShaderProgram, effectsShaderProgram);
		GM_ASSERT(b);

		gm::GMGLShaderProgram* forwardShaderProgram = new gm::GMGLShaderProgram(context);
		gm::GMGLShaderProgram* deferredShaderPrograms[2] = { new gm::GMGLShaderProgram(context), new gm::GMGLShaderProgram(context) };
		initLoadShaderProgram(forwardShaderProgram, deferredShaderPrograms);
		b = context->engine->setInterface(gm::GameMachineInterfaceID::GLForwardShaderProgram, forwardShaderProgram);
		GM_ASSERT(b);

		b = context->engine->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderGeometryProgram, deferredShaderPrograms[0]);
		GM_ASSERT(b);

		b = context->engine->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderLightProgram, deferredShaderPrograms[1]);
		GM_ASSERT(b);
	}
	else
	{
#if GM_USE_DX11
		GM_ASSERT(env == gm::GMRenderEnvironment::DirectX11);
		gm::GMDx11Helper::GMLoadDx11Shader(context->engine, L"dx11/effect.fx", gm::GMShaderType::Effect);
#else
		GM_ASSERT(false);
#endif
	}
}

void DemostrationEntrance::initLoadEffectsShader(gm::GMGLShaderProgram* effectsShaderProgram)
{
	GM_ASSERT(effectsShaderProgram);
	gm::GMBuffer vertBuf, fragBuf;
	gm::GMString vertPath, fragPath;
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/effects/effects.vert", &vertBuf, &vertPath);
	GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/effects/effects.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	gm::GMGLShaderInfo shadersInfo[] = {
		{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)vertBuf.buffer, vertPath },
		{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)fragBuf.buffer, fragPath },
	};

	effectsShaderProgram->attachShader(shadersInfo[0]);
	effectsShaderProgram->attachShader(shadersInfo[1]);
}

void DemostrationEntrance::initLoadShaderProgram(gm::GMGLShaderProgram* forwardShaderProgram, gm::GMGLShaderProgram* deferredShaderProgram[2])
{
	GM_ASSERT(forwardShaderProgram);
	GM_ASSERT(deferredShaderProgram[0]);
	GM_ASSERT(deferredShaderProgram[1]);

	{
		gm::GMBuffer vertBuf, fragBuf;
		gm::GMString vertPath, fragPath;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/main.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/main.frag", &fragBuf, &fragPath);
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		gm::GMGLShaderInfo shadersInfo[] = {
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)vertBuf.buffer, vertPath },
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)fragBuf.buffer, fragPath },
		};

		forwardShaderProgram->attachShader(shadersInfo[0]);
		forwardShaderProgram->attachShader(shadersInfo[1]);
	}

	{
		gm::GMBuffer vertBuf, fragBuf;
		gm::GMString vertPath, fragPath;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/deferred/geometry_pass_main.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/deferred/geometry_pass_main.frag", &fragBuf, &fragPath);
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		gm::GMGLShaderInfo shadersInfo[] = {
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)vertBuf.buffer, vertPath },
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)fragBuf.buffer, fragPath },
		};

		(*deferredShaderProgram[0]).attachShader(shadersInfo[0]);
		(*deferredShaderProgram[0]).attachShader(shadersInfo[1]);
	}

	{
		gm::GMBuffer vertBuf, fragBuf;
		gm::GMString vertPath, fragPath;
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/deferred/light_pass_main.vert", &vertBuf, &vertPath);
		GM.getGamePackageManager()->readFile(gm::GMPackageIndex::Shaders, "gl/deferred/light_pass_main.frag", &fragBuf, &fragPath);
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		gm::GMGLShaderInfo shadersInfo[] = {
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), (const char*)vertBuf.buffer, vertPath },
			{ gm::GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), (const char*)fragBuf.buffer, fragPath },
		};

		(*deferredShaderProgram[1]).attachShader(shadersInfo[0]);
		(*deferredShaderProgram[1]).attachShader(shadersInfo[1]);
	}
}