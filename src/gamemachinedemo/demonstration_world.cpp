#include "stdafx.h"
#include "demonstration_world.h"
#include <gmwidget.h>
#include <gmcontrols.h>
#include <gmcontroltextedit.h>
#include <gmgl.h>
#include <gmgraphicengine.h>
#include <gmrendertechnique.h>
#include <gmglhelper.h>
#include <gmmodelreader.h>
#include <gmtools.h>

#include "demo/texture.h"
#include "demo/normalmap.h"
#include "demo/effects.h"
#include "demo/quake3_bsp.h"
#include "demo/sound.h"
#include "demo/literature.h"
#include "demo/model.h"
#include "demo/collision.h"
#include "demo/specularmap.h"
#include "demo/pbr.h"
#include "demo/phong_pbr.h"
#include "demo/controls.h"
#include "demo/lua.h"
#include "demo/particle.h"
#include "demo/md5mesh.h"
#include "demo/terrain.h"
#include "demo/customshader.h"

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif

extern gm::GMRenderEnvironment GetRenderEnv();

namespace
{
	void loadDemostrations(DemonstrationWorld* world)
	{
		world->addDemo(L"入门: 读取一个纹理。", new Demo_Texture(world));
		world->addDemo(L"入门: 使用索引缓存读取一个纹理。", new Demo_Texture_Index(world));
		world->addDemo(L"高级纹理：使用法线贴图。", new Demo_NormalMap(world));
		world->addDemo(L"效果：使用自带的各种滤镜。", new Demo_Effects(world));
		world->addDemo(L"BSP: 渲染一个雷神之锤3的场景。", new Demo_Quake3_BSP(world));
		world->addDemo(L"声音: 演示播放音乐。", new Demo_Sound(world));
		//world->addDemo(L"文字: 使用排版引擎实现排版。", new Demo_Literature(world));
		world->addDemo(L"模型: 读取模型文件。", new Demo_Model(world));
		world->addDemo(L"物理: 演示相互碰撞的物体。", new Demo_Collision(world));
		world->addDemo(L"高光贴图: 演示一个带有高光贴图的立方体。", new Demo_SpecularMap(world));
		world->addDemo(L"PBR: 演示PBR渲染。", new Demo_PBR(world));
		world->addDemo(L"PBR: 演示同时通过Phong模型和PBR模型渲染。", new Demo_Phong_PBR(world));
		world->addDemo(L"UI控件: 演示使用GameMachine提供的UI控件。", new Demo_Controls(world));
		world->addDemo(L"粒子系统: 渲染一个粒子系统。", new Demo_Particle(world));
		world->addDemo(L"MD5: 渲染MD5骨骼动画。", new Demo_MD5Mesh(world));
		world->addDemo(L"地形: 渲染一个地形。", new Demo_Terrain(world));
		world->addDemo(L"自定义着色器: 使用自定义着色器进行渲染。", new Demo_CustomShader(world));
		world->addDemo(L"LUA: 执行Lua脚本。", new Demo_Lua(world));
		world->init();
	}
}

GM_DEFINE_SIGNAL(DemoHandler::renderingChanged);
GM_DEFINE_SIGNAL(DemoHandler::gammaCorrectionChanged);

DemoHandler::DemoHandler(DemonstrationWorld* parentDemonstrationWorld)
{
	D(d);
	d->parentDemonstrationWorld = parentDemonstrationWorld;
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
	d->engine = parentDemonstrationWorld->getContext()->getEngine();
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
	d->engine->removeLights();
	setDefaultLights();
	d->activating = true;

	gm::GMWidget* widget = getWidget();
	if (widget)
		widget->setVisible(true);
}

void DemoHandler::onDeactivate()
{
	D(d);
	d->parentDemonstrationWorld->resetCameraAndLights();

	d->renderConfig.set(gm::GMRenderConfigs::FilterMode, gm::GMFilterMode::None);
	d->activating = false;

	gm::GMWidget* widget = getWidget();
	if (widget)
	{
		widget->setVisible(false);
		gm::GMWidget::clearFocus(widget);
	}

	gm::GMShadowSourceDesc noShadow;
	noShadow.type = gm::GMShadowSourceDesc::NoShadow;
	d->engine->setShadowSource(noShadow);

	gm::GMRenderConfig config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	config.set(gm::GMRenderConfigs::HDR_Bool, false);

	d->parentDemonstrationWorld->getMainWidget()->setVisible(true);
	d->parentDemonstrationWorld->getBillboardWidget()->setVisible(true);
}

void DemoHandler::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	static const gm::GMString s_fwd = L"渲染模式: 正向渲染";
	static const gm::GMString s_dfr = L"渲染模式: 优先延迟渲染";
	static const gm::GMString s_gammaOn = L"Gamma校正: 开";
	static const gm::GMString s_gammaOff = L"Gamma校正: 关";
	static const gm::GMString s_normalOff = L"查看法向量：无";
	static const gm::GMString s_normalWorldSpace = L"查看法向量：世界空间";
	static const gm::GMString s_normalEyeSpace = L"查看法向量：眼睛空间";

	switch (evt)
	{
	case gm::GameMachineHandlerEvent::Render:
	{
		if (d->lbFPS)
			d->lbFPS->setText(gm::GMString(L"当前FPS: ") + gm::GMString(GM.getRunningStates().fps));

		if (d->lbRendering)
			d->lbRendering->setText(
				(getDemoWorldReference()->getRenderPreference() == gm::GMRenderPreference::PreferForwardRendering ? s_fwd : s_dfr)
			);

		if (d->lbGammaCorrection)
			d->lbGammaCorrection->setText(
				(d->renderConfig.get(gm::GMRenderConfigs::GammaCorrection_Bool).toBool() ? s_gammaOn : s_gammaOff)
			);

		if (d->lbDebugNormal)
		{
			auto mode = d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonNormalMode).toInt();
			if (mode == gm::GMDrawPolygonNormalMode::Off)
				d->lbDebugNormal->setText(s_normalOff);
			else if (mode == gm::GMDrawPolygonNormalMode::WorldSpace)
				d->lbDebugNormal->setText(s_normalWorldSpace);
			else if (mode == gm::GMDrawPolygonNormalMode::EyeSpace)
				d->lbDebugNormal->setText(s_normalEyeSpace);
		}

		break;
	}
	case gm::GameMachineHandlerEvent::Activate:
		gm::IInput* inputManager = getDemonstrationWorld()->getMainWindow()->getInputMananger();
		gm::IKeyboardState& kbState = inputManager->getKeyboardState();

		if (kbState.keyTriggered(gm::GMKey_Escape))
			backToEntrance();

		if (kbState.keyTriggered(gm::GM_ASCIIToKey('I')))
			d->debugConfig.set(gm::GMDebugConfigs::RunProfile_Bool, !d->debugConfig.get(gm::GMDebugConfigs::RunProfile_Bool).toBool());

		break;
	}
}

gm::GMWidget* DemoHandler::getWidget()
{
	D(d);
	return d->mainWidget.get();
}

void DemoHandler::setLookAt()
{
	gm::GMCamera& camera = getDemonstrationWorld()->getContext()->getEngine()->getCamera();
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

const gm::GMString& DemoHandler::getDescription() const
{
	static gm::GMString empty;
	return empty;
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

gm::GMWidget* DemoHandler::createDefaultWidget()
{
	D(d);
	gm::GMFontHandle stxingka = d->engine->getGlyphManager()->addFontByFileName("STXINGKA.TTF");
	d->mainWidget = gm_makeOwnedPtr<gm::GMWidget>(getDemonstrationWorld()->getManager());
	getDemonstrationWorld()->getManager()->registerWidget(d->mainWidget.get());
	getDemonstrationWorld()->getUIConfiguration()->initWidget(d->mainWidget.get());

	d->mainWidget->setTitle(L"选项菜单");
	d->mainWidget->setTitleVisible(true);
	d->mainWidget->setVerticalScrollbarWidth(20);
	gm::GMRect thumbCorner = { 0, 0, 7, 15 };
	d->mainWidget->setScrollbarThumbCorner(thumbCorner);

	if (stxingka != gm::GMInvalidFontHandle)
	{
		gm::GMStyle style = d->mainWidget->getTitleStyle();
		style.setFont(stxingka);
		d->mainWidget->setTitleStyle(style);
	}

	d->mainWidget->setKeyboardInput(true);
	d->mainWidget->setVisible(false);

	gm::GMRect corner = { 0,0,75,42 };
	d->mainWidget->addBorder(corner);
	d->mainWidget->setPosition(10, 60);
	d->mainWidget->setSize(300, 500);
	getDemoWorldReference()->getContext()->getWindow()->addWidget(d->mainWidget.get());

	gm::GMint32 top = 10;

	if (!getDescription().isEmpty())
	{
		d->mainWidget->addControl(gm::GMControlLabel::createControl(
			d->mainWidget.get(),
			getDescription(),
			getLabelFontColor(),
			10,
			top,
			250,
			30,
			false
		));
		top += 40;
	}

	gm::GMControlButton* button = nullptr;
	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"返回主菜单",
		10,
		top,
		250,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		backToEntrance();
	});

	d->mainWidget->addControl(gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		L"当前状态:",
		getLabelFontColor(),
		10,
		top += 40,
		250,
		20,
		false
	));

	d->mainWidget->addControl(d->lbFPS = gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		L"",
		getLabelFontColor(),
		10,
		top += 20,
		250,
		20,
		false
	));

	d->mainWidget->addControl(d->lbRendering = gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		L"",
		getLabelFontColor(),
		10,
		top += 20,
		250,
		20,
		false
	));

	d->mainWidget->addControl(d->lbGammaCorrection = gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		L"",
		getLabelFontColor(),
		10,
		top += 20,
		250,
		20,
		false
	));

	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"开启/关闭延迟渲染",
		10,
		top += 40,
		250,
		30,
		false
	));

	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		if (getDemoWorldReference()->getRenderPreference() == gm::GMRenderPreference::PreferDeferredRendering)
			getDemoWorldReference()->setRenderPreference(gm::GMRenderPreference::PreferForwardRendering);
		else
			getDemoWorldReference()->setRenderPreference(gm::GMRenderPreference::PreferDeferredRendering);
	});

	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"开启/关闭Gamma校正",
		10,
		top += 40,
		250,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->renderConfig.set(gm::GMRenderConfigs::GammaCorrection_Bool, !d->renderConfig.get(gm::GMRenderConfigs::GammaCorrection_Bool).toBool());
	});

	d->mainWidget->addControl(d->lbDebugNormal = gm::GMControlLabel::createControl(
		d->mainWidget.get(),
		"",
		getLabelFontColor(),
		10,
		top += 40,
		250,
		30,
		false
	));

	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"查看法向量",
		10,
		top += 40,
		250,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		switchNormal();
	});

	d->mainWidget->addControl(button = gm::GMControlButton::createControl(
		d->mainWidget.get(),
		L"切换线框模式",
		10,
		top += 40,
		250,
		30,
		false
	));
	button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		d->debugConfig.set(gm::GMDebugConfigs::WireFrameMode_Bool, !d->debugConfig.get(gm::GMDebugConfigs::WireFrameMode_Bool).toBool());
	});

	d->nextControlTop = top + 40;
	d->mainWidget->setSize(d->mainWidget->getSize().width, d->nextControlTop);

	return d->mainWidget.get();
}

gm::GMint32 DemoHandler::getClientAreaTop()
{
	D(d);
	return d->nextControlTop;
}

DemonstrationWorld::DemonstrationWorld(const gm::IRenderContext* context, gm::IWindow* window)
	: Base(context)
{
	D(d);
	d->mainWindow = window;
	d->configuration.reset(new gm::GMUIConfiguration(context));
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
	GM_delete(d->billboard);
	GM_delete(d->manager);
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
	gm::IGraphicEngine* engine = getContext()->getEngine();
	engine->getDefaultFramebuffers()->setClearColor(ValuePointer(gm::GMConvertion::hexToRGB(L"#041B4B")));
	gm::GMFontHandle stxingka = engine->getGlyphManager()->addFontByFileName("STXINGKA.TTF");

	/*
	{
		gm::GMPoint p1 = { 0, 0 }, p2 = { 200, 300 };
		getPrimitiveManager()->addLine(p1, p2, GMVec4(0, 1, 0, 1));
	}
	{
		gm::GMPoint p1 = { 200, 300 }, p2 = { 300, 200 };
		getPrimitiveManager()->addLine(p1, p2, GMVec4(0, 1, 0, 1));
	}
	*/

	// 创建Widget
	gm::GMBuffer ui;
	package->readFile(gm::GMPackageIndex::Textures, L"skin.gmskin", &ui);
	d->configuration->import(ui);

	d->manager = new gm::GMWidgetResourceManager(getContext());
	d->mainWidget = new gm::GMWidget(d->manager);
	d->configuration->initResourceManager(d->manager);
	d->configuration->initWidget(d->mainWidget);

	d->manager->registerWidget(d->mainWidget);
	d->mainWidget->setPosition(10, 60);
	d->mainWidget->setSize(500, 650);
	d->mainWidget->setTitle(L"GameMachine - 展示菜单");
	d->mainWidget->setTitleVisible(true);
	d->mainWidget->setKeyboardInput(true);
	d->mainWidget->setVerticalScrollbarWidth(20);
	gm::GMRect thumbCorner = { 0, 0, 7, 15 };
	d->mainWidget->setScrollbarThumbCorner(thumbCorner);

	gm::GMint32 Y = 10, marginY = 10;
	for (auto& demo : d->demos)
	{
		gm::GMControlButton* button = nullptr;
		d->mainWidget->addControl(button = gm::GMControlButton::createControl(
			d->mainWidget,
			demo.first,
			10,
			Y,
			450,
			30,
			false
		));
		Y += 30 + marginY;
		GM_ASSERT(button);
		button->connect(*button, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
			d->nextDemo = demo.second;
			getMainWidget()->setVisible(false);
			getBillboardWidget()->setVisible(false);
		});
	}

	gm::GMControlButton* exitButton = nullptr;
	d->mainWidget->addControl(exitButton = gm::GMControlButton::createControl(
		d->mainWidget,
		L"退出程序",
		10,
		Y,
		450,
		30,
		false
	));
	Y += 30 + marginY;
	GM_ASSERT(exitButton);
	exitButton->connect(*exitButton, GM_SIGNAL(gm::GMControlButton::click), [=](gm::GMObject* sender, gm::GMObject* receiver) {
		GM.exit();
	});

	gm::GMRect corner = { 0,0,75,42 };
	d->mainWidget->addBorder(corner);
	d->mainWindow->addWidget(d->mainWidget);

	gm::GMWidget* billboard = new gm::GMWidget(d->manager);
	d->configuration->initWidget(billboard);
	billboard->setSize(270, 90);
	billboard->setPosition(600, 60);
	gm::GMControlLabel* lb = gm::GMControlLabel::createControl(
		billboard,
		L"欢迎来到GameMachine的世界。\n"
		L"在这里，你将看到GameMachine所展示出来的部分功能。",
		GMVec4(1, 1, 1, 1),
		10,
		0,
		250,
		80,
		false
	);
	billboard->addControl(lb);
	billboard->addBorder(corner);
	d->billboard = billboard;
	d->mainWindow->addWidget(billboard);

	initObjects();
}

void DemonstrationWorld::initObjects()
{
	D(d);
	gm::GMGamePackage& pk = *GM.getGamePackageManager();
	gm::GMModelLoadSettings loadSettings(
		"love/love.obj",
		"love",
		getContext()
	);

	gm::GMAsset models;
	bool b = gm::GMModelReader::load(loadSettings, models);
	GM_ASSERT(b);
	d->logoObj = new gm::GMGameObject(models);
	d->logoObj->setScaling(Scale(GMVec3(.01f, .01f, .01f)));
	d->logoObj->setTranslation(Translate(GMVec3(0, -.2f, 0)));

	// 创建动画
	GMFloat4 t4, s4;
	GetTranslationFromMatrix(d->logoObj->getTranslation(), t4);
	GetScalingFromMatrix(d->logoObj->getScaling(), s4);
	GMVec4 t, s;
	t.setFloat4(t4);
	s.setFloat4(s4);
	d->logoAnimation.setTargetObjects(d->logoObj);
	d->logoAnimation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 2 / 3, (GMVec3(0, 1, 0)))),
		1.5f
	));
	d->logoAnimation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 4 / 3, (GMVec3(0, 1, 0)))),
		3.f
	));
	d->logoAnimation.addKeyFrame(new gm::GMGameObjectKeyframe(
		t,
		s,
		(Rotate(PI * 2, (GMVec3(0, 1, 0)))),
		4.5f
	));
	d->logoAnimation.setPlayLoop(true);
	d->logoAnimation.play();

	// 加入容器
	addObjectAndInit(d->logoObj);
	addToRenderList(d->logoObj);
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

void DemonstrationWorld::resetCameraAndLights()
{
	D(d);
	auto engine = getContext()->getEngine();
	engine->removeLights();

	// 设置一个默认视角
	gm::GMCamera& camera = getContext()->getEngine()->getCamera();
	camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, 1 };
	lookAt.position = { 0, 0, -1 };
	camera.lookAt(lookAt);

	{
		gm::ILight* light = nullptr;
		GM.getFactory()->createLight(gm::GMLightType::Ambient, &light);
		gm::GMfloat lightClr[] = { .1f, .1f, .1f, 1 };
		light->setLightColor(lightClr);
		engine->addLight(light);
	}

	{
		gm::ILight* light = nullptr;
		GM.getFactory()->createLight(gm::GMLightType::Direct, &light);
		gm::GMfloat lightPos[] = { -5, 5, -5 };
		light->setLightPosition(lightPos);
		gm::GMfloat lightClr[] = { 2, 2, 2, 1 };
		light->setLightColor(lightClr);
		engine->addLight(light);
	}
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init(const gm::IRenderContext* context)
{
	D(d);
	auto rc = context->getWindow()->getRenderRect();
	gm::GMGamePackage* pk = GM.getGamePackageManager();

#if GM_WINDOWS
#	ifdef GM_DEBUG
	pk->loadPackage("D:/gmpk");
#	else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#else
#	ifdef GM_DEBUG
	pk->loadPackage("/home/froser/Documents/gmpk");
#	else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#	endif
#endif

	{
		gm::GMBuffer buf;
		gm::GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(gm::GMPackageIndex::Fonts, L"simhei.ttf", &buf);
		gm::GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setCN(font);
	}

	{
		gm::GMBuffer buf;
		gm::GMGlyphManager* glyphManager = context->getEngine()->getGlyphManager();
		pk->readFile(gm::GMPackageIndex::Fonts, L"times.ttf", &buf);
		gm::GMFontHandle font = glyphManager->addFontByMemory(std::move(buf));
		glyphManager->setEN(font);
	}

	context->getEngine()->setShaderLoadCallback(this);
	d->renderConfig = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(gm::GMConfigs::Debug).asDebugConfig();
	d->world = new DemonstrationWorld(context, d->mainWindow);
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = d->mainWindow->getInputMananger();
	getWorld()->resetCameraAndLights();
	loadDemostrations(d->world);
}

void DemostrationEntrance::event(gm::GameMachineHandlerEvent evt)
{
	D(d);
	if (evt == gm::GameMachineHandlerEvent::Render)
	{
		gm::IGraphicEngine* engine = getWorld()->getContext()->getEngine();
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
		case gm::GameMachineHandlerEvent::Update:
			getWorld()->getLogoAnimation().update(GM.getRunningStates().lastFrameElpased);
			break;
		case gm::GameMachineHandlerEvent::Render:
			getWorld()->renderScene();
			getWorld()->getPrimitiveManager()->render();
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

void DemostrationEntrance::onLoadShaders(const gm::IRenderContext* context)
{
	D(d);
	Demo_CustomShader::initCustomShader(context);
	auto& env = GM.getRunningStates().renderEnvironment;
	if (env == gm::GMRenderEnvironment::OpenGL)
	{
		gm::GMGLHelper::loadShader(
			context,
			L"gl/main.vert",
			L"gl/main.frag",
			L"gl/deferred/geometry_pass_main.vert",
			L"gl/deferred/geometry_pass_main.frag",
			L"gl/deferred/light_pass_main.vert",
			L"gl/deferred/light_pass_main.frag",
			L"gl/filters/filters.vert",
			L"gl/filters/filters.frag"
		);
	}
	else
	{
#if GM_USE_DX11
		gm::GMDx11Helper::loadShader(context, L"dx11/effect.fx");
#else
		GM_ASSERT(false);
#endif
	}
}