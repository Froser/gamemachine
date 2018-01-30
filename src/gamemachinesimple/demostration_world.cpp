#include "stdafx.h"
#include "demostration_world.h"
#include <gmcontrolgameobject.h>
#include <gmdx11helper.h>
#include <gmcom.h>
#include <gmdxincludes.h>

extern gm::GMRenderEnvironment GetRenderEnv();

namespace
{
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
	d->activating = true;
}

void DemoHandler::onDeactivate()
{
	D(d);
	d->demostrationWorld->resetProjectionAndEye();
	GM.getGraphicEngine()->removeLights();
	GMSetRenderState(EFFECTS, gm::GMEffects::None);
	d->activating = false;
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
	gm::GMCameraLookAt lookAt;
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

bool DemoHandler::isActivating()
{
	D(d);
	return d->activating;
}

DemostrationWorld::~DemostrationWorld()
{
	D(d);
	for (auto& demo : d->demos)
	{
		GM_ASSERT(demo.second);
		gm::GM_delete(demo.second);
	}
	gm::GM_delete(d->cursor);
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
	//创建光标
	d->cursor = new gm::GMCursorGameObject(32, 32);

	gm::ITexture* curFrame = nullptr;
	gm::GMTextureUtil::createTexture("cursor.png", &curFrame);
	GM_ASSERT(curFrame);

	gm::GMAsset curAsset = getAssets().insertAsset(gm::GMAssetType::Texture, curFrame);
	d->cursor->setImage(curAsset);
	d->cursor->enableCursor();
	GM.setCursor(d->cursor);

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

void DemostrationWorld::renderScene()
{
	D(d);
	Base::renderScene();

	gm::IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginBlend();
	auto controls = getControlsGameObject();
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

void DemostrationWorld::resetProjectionAndEye()
{
	// 设置一个默认视角
	gm::GMCamera& camera = GM.getCamera();
	camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);

	gm::GMCameraLookAt lookAt;
	lookAt.lookAt = { 0, 0, -1 };
	lookAt.position = { 0, 0, 1 };
	camera.lookAt(lookAt);
}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init()
{
	D(d);
	auto& rc = GM.getGameMachineRunningStates().clientRect;

	gm::GMGamePackage* pk = GM.getGamePackageManager();

#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((gm::GMPath::getCurrentPath() + L"gm.pk0"));
#endif

	GM.getGraphicEngine()->setShaderLoadCallback(this);

	GMSetRenderState(RENDER_MODE, gm::GMStates_RenderOptions::DEFERRED);
	//GMSetRenderState(EFFECTS, GMEffects::Grayscale);
	GMSetRenderState(RESOLUTION_X, rc.width);
	GMSetRenderState(RESOLUTION_Y, rc.height);

	d->world = new DemostrationWorld();
}

void DemostrationEntrance::start()
{
	D(d);
	gm::IInput* inputManager = GM.getMainWindow()->getInputMananger();
	getWorld()->resetProjectionAndEye();
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

	if (GM.getCursor())
	{
		if (evt == gm::GameMachineEvent::FrameStart)
			GM.getCursor()->update();
		else if (evt == gm::GameMachineEvent::Render)
			GM.getCursor()->drawCursor();
	}
}

DemostrationEntrance::~DemostrationEntrance()
{
	D(d);
	gm::GM_delete(d->world);
}

void DemostrationEntrance::onLoadShaders(gm::IGraphicEngine* engine)
{
	HRESULT hr = gm::GMLoadDx11Shader(L"dx11/color.vs", L"ColorVertexShader", L"vs_5_0", gm::GM_VERTEX_SHADER);
	GM_COM_CHECK(hr);
	hr = gm::GMLoadDx11Shader(L"dx11/color.ps", L"ColorPixelShader", L"ps_5_0", gm::GM_PIXEL_SHADER);
	GM_COM_CHECK(hr);
}