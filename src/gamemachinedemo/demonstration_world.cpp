#include "stdafx.h"
#include "demonstration_world.h"
#include <gmcontrolgameobject.h>
#include <gmgl.h>

#include "demo/texture.h"
#include "demo/normalmap.h"
#include "demo/effects.h"
#include "demo/quake3_bsp.h"
#include "demo/border.h"
#include "demo/sound.h"
#include "demo/literature.h"
#include "demo/button.h"
#include "demo/model.h"
#include "demo/collision.h"
#include "demo/specularmap.h"

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif

extern gm::GMRenderEnvironment GetRenderEnv();

namespace
{
	void loadDemostrations(DemostrationWorld* world)
	{
		world->addDemo("Hello World: Load a texture", new Demo_Texture(world));
		world->addDemo("Hello World: Load a texture with indices buffer", new Demo_Texture_Index(world));
		world->addDemo("Texture advance: Load texture with normal map", new Demo_NormalMap(world));
		world->addDemo("Effects: Use a grayscale filter.", new Demo_Effects(world));
		world->addDemo("BSP: Demonstrate a Quake3 scene.", new Demo_Quake3_BSP(world));
		world->addDemo("Border: Demonstrate a border.", new Demo_Border(world));
		world->addDemo("Sound: Demonstrate playing music.", new Demo_Sound(world));
		world->addDemo("Literature: Demonstrate render literatures via GMTypoEngine.", new Demo_Literature(world));
		world->addDemo("Button: Demonstrate how to create a button.", new Demo_Button(world));
		world->addDemo("Model: Load a model. Adjust model by dragging or wheeling.", new Demo_Model(world));
		world->addDemo("Physics: Demonstrate collision objects.", new Demo_Collision(world));
		world->addDemo("SpecularMap: Demonstrate a cube with specular map.", new Demo_SpecularMap(world));
		world->init();
	}
}

DemoHandler::DemoHandler(DemostrationWorld* parentDemonstrationWorld)
{
	D(d);
	d->parentDemonstrationWorld = parentDemonstrationWorld;
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
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
	GM.getGraphicEngine()->removeLights();
	d->renderConfig.set(gm::GMRenderConfigs::FilterMode, gm::GMFilterMode::None);
	d->activating = false;

	gm::GMShadowSourceDesc noShadow;
	noShadow.type = gm::GMShadowSourceDesc::NoShadow;
	GM.getGraphicEngine()->setShadowSource(noShadow);
}

void DemoHandler::event(gm::GameMachineEvent evt)
{
	D(d);
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
		GM.getGraphicEngine()->addLight(light);
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
	gm::GMGamePackage* package = GM.getGamePackageManager();

	gm::GMListbox2DGameObject* listbox = new gm::GMListbox2DGameObject();

	// 读取边框
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

void DemostrationWorld::resetProjectionAndEye()
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
void DemostrationEntrance::init()
{
	D(d);
	auto& rc = GM.getGameMachineRunningStates().renderRect;

	gm::GMGamePackage* pk = GM.getGamePackageManager();

#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#endif

	GM.getGraphicEngine()->setShaderLoadCallback(this);
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
	d->world = new DemostrationWorld();
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	getWorld()->resetProjectionAndEye();
	loadDemostrations(d->world);
}

void DemostrationEntrance::event(gm::GameMachineEvent evt)
{
	D(d);
	if (evt == gm::GameMachineEvent::Render)
	{
		gm::IGraphicEngine* engine = GM.getGraphicEngine();
		engine->getDefaultFramebuffers()->clear();
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
				d->debugConfig.set(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool, !d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonsAsLine_Bool).toBool());

			if (kbState.keyTriggered('I'))
				d->debugConfig.set(gm::GMDebugConfigs::RunProfile_Bool, !d->debugConfig.get(gm::GMDebugConfigs::RunProfile_Bool).toBool());

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

void DemostrationEntrance::onLoadShaders(gm::IGraphicEngine* engine)
{
	auto& env = GM.getGameMachineRunningStates().renderEnvironment;

	if (env == gm::GMRenderEnvironment::OpenGL)
	{
		bool b;
		gm::GMGLShaderProgram* effectsShaderProgram = new gm::GMGLShaderProgram();
		initLoadEffectsShader(effectsShaderProgram);
		b = engine->setInterface(gm::GameMachineInterfaceID::GLEffectShaderProgram, effectsShaderProgram);
		GM_ASSERT(b);

		gm::GMGLShaderProgram* forwardShaderProgram = new gm::GMGLShaderProgram();
		gm::GMGLShaderProgram* deferredShaderPrograms[2] = { new gm::GMGLShaderProgram(), new gm::GMGLShaderProgram() };
		initLoadShaderProgram(forwardShaderProgram, deferredShaderPrograms);
		b = engine->setInterface(gm::GameMachineInterfaceID::GLForwardShaderProgram, forwardShaderProgram);
		GM_ASSERT(b);

		b = engine->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderGeometryProgram, deferredShaderPrograms[0]);
		GM_ASSERT(b);

		b = engine->setInterface(gm::GameMachineInterfaceID::GLDeferredShaderLightProgram, deferredShaderPrograms[1]);
		GM_ASSERT(b);
	}
	else
	{
#if GM_USE_DX11
		GM_ASSERT(env == gm::GMRenderEnvironment::DirectX11);
		gm::GMDx11Helper::GMLoadDx11Shader(GM.getGraphicEngine(), L"dx11/effect.fx", gm::GMShaderType::Effect);
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