#define GLEW_STATIC
#define FREEGLUT_STATIC
#include <windows.h>
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"
#include "gmgl/gmglfactory.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gmbspgameworld.h"
#include "foundation/debug.h"
#include "os/gminput.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"

#include <fstream>
#include "gmdatacore/soundreader/gmsoundreader.h"
#include "gmui/gmui.h"
#include "gmui/gmui_glwindow.h"
#include "gmengine/gmdemogameworld.h"
#include "gmengine/gmspritegameobject.h"
#include "gmdatacore/imagereader/gmimagereader.h"
#include "gmengine/gmparticles.h"
#include "foundation/utilities/gmprimitivecreator.h"
#include "gmlua/gmlua.h"

#define EMITTER_DEMO 1

using namespace gm;

GMGLFactory factory;
//ISoundFile* sf;

// 这是一个导出所有资源的钩子，用gm_install_hook(GMGamePackage, readFileFromPath, resOutputHook)绑定此钩子
// 可以将所有场景中的资源导出到指定目录
static void resOutputHook(void* path, void* buffer)
{
	const char* resPath = (const char*)path;
	GMBuffer* buf = (GMBuffer*)buffer;
	if (buf->size == 0)
		return;

	std::fstream out;
	GMString p = std::string("D:/output/") + resPath;
	GMString dir = Path::directoryName(p);
	dir = dir.substr(0, dir.length() - 1);
	Path::createDirectory(dir);

	out.open(p.toStdWString().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (out.good())
	{
		GMint sz = buf->size;
		out.seekg(0, std::ios::beg);
		out.write((char*)buf->buffer, sz);
		out.close();
	}
}

class GameHandler : public IGameHandler, public IShaderLoadCallback
{
public:
	GameHandler()
		: m_bMouseEnable(true)
	{
	}

	void init()
	{
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*> (GameMachine::instance().getGraphicEngine());
		engine->setShaderLoadCallback(this);
		engine->setRenderMode(GMGLRenderMode::DeferredRendering);

		GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
#ifdef _DEBUG
		pk->loadPackage("D:/gmpk");
#else
		pk->loadPackage((Path::getCurrentPath() + _L("gm.pk0")));
#endif

	}

	void start()
	{
		//gm_install_hook(GMGamePackage, readFileFromPath, resOutputHook);
		IInput* inputManager = GameMachine::instance().getInputManager();
		inputManager->getMouseState().initMouse(GameMachine::instance().getMainWindow());
		inputManager->getKeyboardState().setIMEState(false);

		GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
		pk->createBSPGameWorld("gv.bsp", &m_world);
		m_sprite = static_cast<GMSpriteGameObject*> (const_cast<GMGameObject*> (*(m_world->getGameObjects(GMGameObjectType::Sprite).begin())));

		m_glyph = new GMGlyphObject();
		m_glyph->setGeometry(-1, .8f, 1, 1);
		m_world->appendObjectAndInit(m_glyph, true);

		//GMBuffer bg;
		//pk.readFile(PI_SOUNDS, "bgm/bgm.mp3", &bg);
		//SoundReader::load(bg, &sf);
		//sf->play();
	}

	void event(GameMachineEvent evt)
	{
		switch (evt)
		{
		case GameMachineEvent::Terminate:
			delete m_world;
			break;
		case GameMachineEvent::Simulate:
			{
				m_world->simulateGameWorld();
				// 更新Camera
				GMCamera& camera = GameMachine::instance().getCamera();
				camera.synchronize(m_sprite);
				break;
			}
		case GameMachineEvent::Render:
			{
				IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
				engine->newFrame();

				GMCamera& camera = GameMachine::instance().getCamera();
				camera.apply();
				m_world->renderGameWorld();

				const PositionState& position = m_sprite->getPositionState();
				GMWchar x[32], y[32], z[32], fps[32];
				swprintf_s(x, L"%f", position.position[0]);
				swprintf_s(y, L"%f", position.position[1]);
				swprintf_s(z, L"%f", position.position[2]);
				swprintf_s(fps, L"%f", GameMachine::instance().getFPS());
				std::wstring str;
				str.append(x);
				str.append(L",");
				str.append(y);
				str.append(L",");
				str.append(z);
				str.append(L" fps: ");
				str.append(fps);
				m_glyph->setText(str.c_str());
			}
			break;
		case GameMachineEvent::Activate:
			IInput* inputManager = GameMachine::instance().getInputManager();
			static GMfloat mouseSensitivity = 0.25f;
			static GMfloat joystickSensitivity = 0.0003f;

			IKeyboardState& kbState = inputManager->getKeyboardState();
			IJoystickState& joyState = inputManager->getJoystickState();
			IMouseState& mouseState = inputManager->getMouseState();

			if (kbState.keydown('Q') || kbState.keydown(VK_ESCAPE))
				GameMachine::instance().postMessage({ GameMachineMessageType::OnExit });
			if (kbState.keydown('B'))
				GameMachine::instance().postMessage({ GameMachineMessageType::OnConsole });

			GMMovement moveTag = MC_NONE;
			GMMoveRate rate;
			GMJoystickState state = joyState.joystickState();

			if (kbState.keydown('A'))
				moveTag |= MC_LEFT;
			if (state.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MC_LEFT;
				rate.setMoveRate(MC_LEFT, GMfloat(state.thumbLX) / SHRT_MIN);
			}

			if (kbState.keydown('D'))
				moveTag |= MC_RIGHT;
			if (state.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MC_RIGHT;
				rate.setMoveRate(MC_RIGHT, GMfloat(state.thumbLX) / SHRT_MAX);
			}

			if (kbState.keydown('S'))
				moveTag |= MC_BACKWARD;
			if (state.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MC_BACKWARD;
				rate.setMoveRate(MC_BACKWARD, GMfloat(state.thumbLY) / SHRT_MIN);
			}

			if (kbState.keydown('W'))
				moveTag |= MC_FORWARD;
			if (state.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MC_FORWARD;
				rate.setMoveRate(MC_FORWARD, GMfloat(state.thumbLY) / SHRT_MAX);
			}

			if (kbState.keyTriggered(VK_SPACE) || state.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER || state.buttons & XINPUT_GAMEPAD_LEFT_SHOULDER)
				moveTag |= MC_JUMP;

			if (kbState.keyTriggered('V'))
				joyState.joystickVibrate(30000, 30000);
			else if (kbState.keydown('C'))
				joyState.joystickVibrate(0, 0);

			if (kbState.keyTriggered('N'))
				GMSetBuiltIn(DRAW_NORMAL, (GMGetBuiltIn(DRAW_NORMAL) + 1) % GMConfig_BuiltInOptions::DRAW_NORMAL_END);
			if (kbState.keyTriggered('M'))
				GMSetBuiltIn(DRAW_LIGHTMAP_ONLY, !GMGetBuiltIn(DRAW_LIGHTMAP_ONLY));
			if (kbState.keyTriggered('I'))
				GMSetBuiltIn(RUN_PROFILE, !GMGetBuiltIn(RUN_PROFILE));

			if (state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat) state.thumbRX / (
					state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				m_sprite->lookRight(state.thumbRX * joystickSensitivity * rate);
			}
			if (state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat)state.thumbRY / (
					state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				m_sprite->lookUp(state.thumbRY * joystickSensitivity * rate);
			}

			GMMouseState ms = mouseState.mouseState();
			m_sprite->lookUp(-ms.deltaY * mouseSensitivity);
			m_sprite->lookRight(ms.deltaX * mouseSensitivity);
			m_sprite->action(moveTag, rate);

			if (kbState.keyTriggered('P'))
				GMSetBuiltIn(CALCULATE_BSP_FACE, !GMGetBuiltIn(CALCULATE_BSP_FACE));
			if (kbState.keyTriggered('L'))
				GMSetBuiltIn(POLYGON_LINE_MODE, !GMGetBuiltIn(POLYGON_LINE_MODE));
			if (kbState.keyTriggered('O'))
				GMSetBuiltIn(DRAW_ONLY_SKY, !GMGetBuiltIn(DRAW_ONLY_SKY));
			if (kbState.keyTriggered('R'))
				mouseState.setMouseEnable(m_bMouseEnable = !m_bMouseEnable);
			break;
		}
	}

	bool isWindowActivate()
	{
		GMUIWindow* window = GameMachine::instance().getMainWindow();
		return ::GetForegroundWindow() == window->getWindowHandle();
	}

	bool onLoadForwardShader(const GMMeshType type, GMGLShaderProgram& shader) override
	{
		bool flag = false;
		GMBuffer vertBuf, fragBuf;
		switch (type)
		{
		case GMMeshType::Model:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "object.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "object.frag", &fragBuf);
			flag = true;
			break;
		case GMMeshType::Glyph:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "glyph.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "glyph.frag", &fragBuf);
			flag = true;
			break;
		case GMMeshType::Particles:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "particles.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "particles.frag", &fragBuf);
			flag = true;
			break;
		default:
			break;
		}

		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer },
			{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer },
		};

		shader.attachShader(shadersInfo[0]);
		shader.attachShader(shadersInfo[1]);
		return flag;
	}

	bool onLoadDeferredPassShader(GMGLDeferredRenderState state, GMGLShaderProgram& shaderProgram) override
	{
		GMBuffer vertBuf, fragBuf;
		switch (state)
		{
		case GMGLDeferredRenderState::GeometryPass:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "geometry_pass.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "geometry_pass.frag", &fragBuf);
			break;
		case gm::GMGLDeferredRenderState::PassingMaterial:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "material_pass.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "material_pass.frag", &fragBuf);
			break;
		case gm::GMGLDeferredRenderState::PassingFlags:
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "flag_pass.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "flag_pass.frag", &fragBuf);
			break;
		default:
			return false;
		}
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer },
			{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer },
		};

		shaderProgram.attachShader(shadersInfo[0]);
		shaderProgram.attachShader(shadersInfo[1]);
		return true;
	}

	bool onLoadDeferredLightPassShader(GMGLShaderProgram& lightPassProgram) override
	{
		GMBuffer vertBuf, fragBuf;
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "light_pass.vert", &vertBuf);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "light_pass.frag", &fragBuf);
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer },
			{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer },
		};

		lightPassProgram.attachShader(shadersInfo[0]);
		lightPassProgram.attachShader(shadersInfo[1]);
		return true;
	}

	bool m_bMouseEnable;
	GMSpriteGameObject* m_sprite;
	GMBSPGameWorld* m_world;
	GMGlyphObject* m_glyph;
};

class DemoGameHandler : public GameHandler
{
public:
	DemoGameHandler() {}
	~DemoGameHandler() { if (mask) delete mask; }

private:
	virtual void start()
	{
		GMCamera& camera = GameMachine::instance().getCamera();
		//camera.initOrtho(-1, 1, -1, 1, 0, 500);

		IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
		auto container = engine->getResourceContainer();
		auto& textureContainer = container->getTextureContainer();

		auto pk = GameMachine::instance().getGamePackageManager();
#ifdef _DEBUG
		pk->loadPackage("D:/gmpk");
#else
		pk->loadPackage((Path::getCurrentPath() + _L("gm.pk0")));
#endif

		GMImage* img;
		GMBuffer buf;
		pk->readFile(GMPackageIndex::Textures, "bnp.png", &buf);
		GMImageReader::load(buf.buffer, buf.size, &img);

		ITexture* tex;
		factory.createTexture(img, &tex);

		TextureContainer::TextureItemType item = { "", tex };
		textureContainer.insert(item);

		demo = new GMDemoGameWorld();

		{
			m_glyph = new GMGlyphObject();
			m_glyph->setGeometry(-1, .8f, 1, 1);
			demo->appendObjectAndInit(m_glyph);
		}

		{
			GMfloat extents[] = { .15f, .15f, .15f };
			GMfloat pos[] = { 0, 0, -1.f };
			GMModel* maskModel;
			GMPrimitiveCreator::createQuad(extents, pos, &maskModel);
			mask = new GMGameObject(maskModel);
			GameMachine::instance().initObjectPainter(mask->getModel());
		}
		{
			GMfloat extents[] = { .5f, .5f, .5f };
			GMfloat pos[] = { 0, 0, -1.f };
			GMModel* coreObj;
			GMPrimitiveCreator::createQuad(extents, pos, &coreObj);
			GMGameObject* obj = new GMGameObject(coreObj);

			GMModel* model = obj->getModel();
			Shader& shader = model->getAllMeshes()[0]->getComponents()[0]->getShader();
			shader.setCull(GMS_Cull::CULL);

			shader.getMaterial().kd = linear_math::Vector3(.6f, .2f, .3f);
			shader.getMaterial().ks = linear_math::Vector3(.1f, .2f, .3f);
			shader.getMaterial().ka = linear_math::Vector3(1, 1, 1);
			shader.getMaterial().shininess = 20;

			demo->appendObject("cube", obj);

			{
				auto& frames = shader.getTexture().getTextureFrames(GMTextureType::NORMALMAP, 0);
				frames.setOneFrame(0, tex);
				frames.setFrameCount(1);
			}

			{
				auto& frames = shader.getTexture().getTextureFrames(GMTextureType::DIFFUSE, 0);
				frames.setOneFrame(0, tex);
				frames.setFrameCount(1);
			}

			{
				GMLight light(GMLightType::SPECULAR);
				GMfloat pos[] = { 0, 0, .2f };
				light.setLightPosition(pos);
				GMfloat color[] = { .7f, .7f, .7f };
				light.setLightColor(color);
				demo->addLight(light);
			}

		}

#if EMITTER_DEMO
#if 0
		linear_math::Quaternion start, end;
		start.setRotation(linear_math::Vector3(0, 0, 1), 0.f);
		end.setRotation(linear_math::Vector3(0, 0, 1), 5.f);
		GMLerpParticleEmitter::create(
			50,
			GMParticlePositionType::Free,
			1,
			.01f,
			.1f,
			linear_math::Vector3(0, 0, 0),
			linear_math::Vector3(1, 0, 0),
			linear_math::Vector3(-1.414f / 2, -1.414f / 2, 0),
			linear_math::Vector4(1, 0, 0, 1),
			linear_math::Vector4(0, 1, 0, 0),
			start,
			end,
			0.1f,
			1.f,
			GMParticlesEmitter::InfiniteEmissionTimes,
			&emitter
		);
#else
		linear_math::Quaternion start, end;
		start.setRotation(linear_math::Vector3(0, 0, 1), 0.f);
		end.setRotation(linear_math::Vector3(0, 0, 1), 5.f);
		GMRadiusParticlesEmitter::create(
			50,
			GMParticlePositionType::RespawnAtEmitterPosition,
			1,
			.01f,
			.1f,
			linear_math::Vector3(0, 0, 1),
			3.14159f,
			linear_math::Vector3(0, 0, 0),
			linear_math::Vector3(1, 0, 0),
			linear_math::Vector4(1, 0, 0, 1),
			linear_math::Vector4(0, 1, 0, 0),
			start,
			end,
			0.1f,
			1.f,
			GMParticlesEmitter::InfiniteEmissionTimes,
			&emitter
		);
#endif
		demo->appendObject("particles", emitter);
#endif
		GMBuffer buffer;
		pk->readFile(GMPackageIndex::Scripts, "helloworld.lua", &buffer);

#if !EMITTER_DEMO
		{
			GMfloat extents[] = { .15f, .15f, .15f };
			GMfloat pos[] = { 0, 0, -1.f };
			GMModel* m;
			GMPrimitiveCreator::createQuad(extents, pos, &m, GMMeshType::Particles);
			GMCustomParticlesEmitter* emitter = new GMCustomParticlesEmitter(m);
			emitter->load(buffer);
			demo->appendObject("particles", emitter);
		}
#endif

		CameraLookAt lookAt;
		lookAt.lookAt = { 0, 0, -1 };
		lookAt.position = { 0, 0, 1 };
		engine->updateCameraView(lookAt);
	}

	virtual void event(GameMachineEvent evt)
	{
		static linear_math::Vector3 dir = linear_math::normalize(linear_math::Vector3(0, 0, 1));
		switch (evt)
		{
		case gm::GameMachineEvent::FrameStart:
			break;
		case gm::GameMachineEvent::FrameEnd:
			break;
		case gm::GameMachineEvent::Simulate:
			demo->simulateGameWorld();
#if EMITTER_DEMO
			if (emitter && emitter->isEmissionFinished())
			{
				demo->removeObject(emitter);
				emitter = nullptr;
			}
#endif
			break;
		case gm::GameMachineEvent::Render:
			{
				GMWchar fps[32];
				swprintf_s(fps, L"%f", GameMachine::instance().getFPS());
				std::wstring str;
				str.append(L" fps: ");
				str.append(fps);
				m_glyph->setText(str.c_str());

				IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
				engine->newFrame();

				demo->beginCreateStencil();
				//mask->draw();
				demo->endCreateStencil();

				demo->beginUseStencil(true);

				if (rotate)
					a += .01f;

#if EMITTER_DEMO
				emitter->getEmitterPropertiesReference().position += linear_math::Vector3(.001f, 0, 0);
#endif

				GMGameObject* obj = demo->findGameObject("cube");
				linear_math::Quaternion q;
				q.setRotation(dir, a);
				obj->setRotation(q);
				demo->renderGameWorld();

				demo->endUseStencil();
				break;
			}
		case gm::GameMachineEvent::Activate:
			{
				IInput* inputManager = GameMachine::instance().getInputManager();
				IKeyboardState& kbState = inputManager->getKeyboardState();
				if (kbState.keyTriggered('N'))
					GMSetBuiltIn(DRAW_NORMAL, (GMGetBuiltIn(DRAW_NORMAL) + 1) % GMConfig_BuiltInOptions::DRAW_NORMAL_END);

				if (kbState.keyTriggered('L'))
					GMSetBuiltIn(POLYGON_LINE_MODE, !GMGetBuiltIn(POLYGON_LINE_MODE));

				if (kbState.keydown('Q') || kbState.keydown(VK_ESCAPE))
					GameMachine::instance().postMessage({ GameMachineMessageType::OnExit });

				if (kbState.keyTriggered('P'))
					rotate = !rotate;

				if (kbState.keydown('B'))
					GameMachine::instance().postMessage({ GameMachineMessageType::OnConsole });

				if (kbState.keyTriggered('I'))
					GMSetBuiltIn(RUN_PROFILE, !GMGetBuiltIn(RUN_PROFILE));
			}
			break;
		case gm::GameMachineEvent::Deactivate:
			break;
		case gm::GameMachineEvent::Terminate:
			delete demo;
			break;
		default:
			break;
		}
	}

	virtual bool isWindowActivate()
	{
		return true;
	}

	GMfloat a = 0;
	GMDemoGameWorld* demo;
	GMGameObject* mask = nullptr;
	GMfloat pos[5] = { 0 };
	bool rotate = true;
	GMParticlesEmitter* emitter;
};

GM_PRIVATE_OBJECT(MetaTest)
{
	linear_math::Matrix4x4 mat = linear_math::Matrix4x4::identity();
};

class MetaTest : public GMObject
{
	DECLARE_PRIVATE(MetaTest)

	GM_BEGIN_META_MAP
		GM_META(mat, GMMetaMemberType::Matrix4x4)
	GM_END_META_MAP
};

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char * lpCmdLine,
	int nCmdShow
)
{
	GMUIWindowAttributes attrs =
	{
		NULL,
		L"HELLO",
		0,
		0,
		{ 0, 0, 1024 / 2, 1024 / 2 },
		NULL,
	};

	GameMachine::instance().init(
		hInstance,
		new GMGLFactory(),
		new DemoGameHandler()
	);

#if 0
	GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
	GMBuffer buffer;
	pk->readFile(GMPackageIndex::Scripts, "helloworld.lua", &buffer);
	GMLua lua;
	GMLuaStatus result = lua.loadBuffer(buffer);
	int xx = lua_gettop(lua);
	MetaTest meta;
	while (1)
		lua.call("v", {1});
#else
	GameMachine::instance().startGameMachine();
#endif
	return 0;
}