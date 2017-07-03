#define GLEW_STATIC
#define FREEGLUT_STATIC
#include <windows.h>
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"
#include "gmengine/gmcharacter.h"
#include "gmgl/gmglfactory.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglfunc.h"
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

using namespace gm;

GMBSPGameWorld* world;
GMCharacter* character;
GMGLFactory factory;
GMGlyphObject* glyph;
ISoundFile* sf;

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

	void start()
	{
		//gm_install_hook(GMGamePackage, readFileFromPath, resOutputHook);
		IInput* inputManager = GameMachine::instance().getInputManager();
		inputManager->getMouseState().initMouse(GameMachine::instance().getMainWindow());
		inputManager->getKeyboardState().setIMEState(false);

		GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
#ifdef _DEBUG
		pk->loadPackage("D:/gmpk");
#else
		pk->loadPackage((Path::getCurrentPath() + _L("gm.pk0")));
#endif

		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*> (GameMachine::instance().getGraphicEngine());
		engine->setShaderLoadCallback(this);

		pk->createBSPGameWorld("gv.bsp", &world);

		glyph = new GMGlyphObject();
		glyph->setGeometry(-1, .8f, 1, 1);
		world->appendObjectAndInit(glyph, true);


		//GMBuffer bg;
		//pk.readFile(PI_SOUNDS, "bgm/bgm.mp3", &bg);
		//SoundReader::load(bg, &sf);
		//sf->play();
	}

	void event(GameMachineEvent evt)
	{
		switch (evt)
		{
		case GameMachineEvent::Simulate:
			world->simulateGameWorld();
			break;
		case GameMachineEvent::Render:
			world->renderGameWorld();
			{
				const PositionState& position = world->getMajorCharacter()->getPositionState();
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
				glyph->setText(str.c_str());
			}
			break;
		case GameMachineEvent::Activate:
			IInput* inputManager = GameMachine::instance().getInputManager();
			static GMfloat mouseSensitivity = 0.25f;
			static GMfloat joystickSensitivity = 0.0003f;

			GMCharacter* character = world->getMajorCharacter();
			IKeyboardState& kbState = inputManager->getKeyboardState();
			IJoystickState& joyState = inputManager->getJoystickState();
			IMouseState& mouseState = inputManager->getMouseState();

			if (kbState.keydown('Q') || kbState.keydown(VK_ESCAPE))
				GameMachine::instance().postMessage({ GM_MESSAGE_EXIT });
			if (kbState.keydown('B'))
				GameMachine::instance().postMessage({ GM_MESSAGE_CONSOLE });

			MoveAction moveTag = 0;
			MoveRate rate;
			GMJoystickState state = joyState.joystickState();

			if (kbState.keydown('A'))
				moveTag |= MD_LEFT;
			if (state.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_LEFT;
				rate.setMoveRate(MD_LEFT, GMfloat(state.thumbLX) / SHRT_MIN);
			}

			if (kbState.keydown('D'))
				moveTag |= MD_RIGHT;
			if (state.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_RIGHT;
				rate.setMoveRate(MD_RIGHT, GMfloat(state.thumbLX) / SHRT_MAX);
			}

			if (kbState.keydown('S'))
				moveTag |= MD_BACKWARD;
			if (state.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_BACKWARD;
				rate.setMoveRate(MD_BACKWARD, GMfloat(state.thumbLY) / SHRT_MIN);
			}

			if (kbState.keydown('W'))
				moveTag |= MD_FORWARD;
			if (state.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_FORWARD;
				rate.setMoveRate(MD_FORWARD, GMfloat(state.thumbLY) / SHRT_MAX);
			}

			if (kbState.keyTriggered(VK_SPACE) || state.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
				moveTag |= MD_JUMP;

			if (kbState.keyTriggered('V'))
				joyState.joystickVibrate(30000, 30000);
			else if (kbState.keydown('C'))
				joyState.joystickVibrate(0, 0);

			if (kbState.keyTriggered('N'))
				GMSetBuiltIn(DRAW_NORMAL, (GMGetBuiltIn(DRAW_NORMAL) + 1) % GMConfig_BuiltInOptions::DRAW_NORMAL_END);

			if (kbState.keyTriggered('I'))
				GMSetBuiltIn(RUN_PROFILE, !GMGetBuiltIn(RUN_PROFILE));

			if (state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat) state.thumbRX / (
					state.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				world->getMajorCharacter()->lookRight(state.thumbRX * joystickSensitivity * rate);
			}
			if (state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || state.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat)state.thumbRY / (
					state.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				world->getMajorCharacter()->lookUp(state.thumbRY * joystickSensitivity * rate);
			}

			GMMouseState ms = mouseState.mouseState();
			world->getMajorCharacter()->lookUp(-ms.deltaY * mouseSensitivity);
			world->getMajorCharacter()->lookRight(ms.deltaX * mouseSensitivity);

			character->action(moveTag, rate);

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

	bool onLoadShader(const GMMeshType type, GMGLShaderProgram* shaderProgram) override
	{
		bool flag = false;
		GMBuffer vertBuf, fragBuf;
		switch (type)
		{
		case GMMeshType::Normal:
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "object.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "object.frag", &fragBuf);
			flag = true;
			break;
		case GMMeshType::Sky:
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "sky.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "sky.frag", &fragBuf);
			flag = true;
			break;
		case GMMeshType::Glyph:
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "glyph.vert", &vertBuf);
			GameMachine::instance().getGamePackageManager()->readFile(PI_SHADERS, "glyph.frag", &fragBuf);
			flag = true;
			break;
		default:
			flag = false;
			break;
		}

		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer },
			{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer },
		};

		shaderProgram->attachShader(shadersInfo[0]);
		shaderProgram->attachShader(shadersInfo[1]);
		return flag;
	}

	bool m_bMouseEnable;
};

class DemoGameHandler : public IGameHandler
{
	virtual void start()
	{
		demo = new GMDemoGameWorld();
		GMGameObject* obj;
		GMfloat extents[] = { .25f, .25f, .25f };
		demo->createCube(extents, &obj);
		demo->appendObject("cube", obj);

		LightInfo light;
		light.args[LA_KA] = light.args[LA_KA + 1] = light.args[LA_KA + 2] = 1;
		light.lightColor = { 1,.5,1 };
		light.on = true;
		demo->setDefaultAmbientLight(light);

		IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
		CameraLookAt lookAt;
		lookAt.lookAt = { 0, 0, -1 };
		lookAt.position = { 0, 0, 1 };
		engine->updateCameraView(lookAt);
	}

	virtual void event(GameMachineEvent evt)
	{
		switch (evt)
		{
		case gm::GameMachineEvent::FrameStart:
			break;
		case gm::GameMachineEvent::FrameEnd:
			break;
		case gm::GameMachineEvent::Simulate:
			break;
		case gm::GameMachineEvent::Render:
			demo->renderGameWorld();
			break;
		case gm::GameMachineEvent::Activate:
			break;
		case gm::GameMachineEvent::Deactivate:
			break;
		default:
			break;
		}
	}

	virtual bool isWindowActivate()
	{
		return true;
	}

	GMDemoGameWorld* demo;
};

int main()
{
	WinMain(NULL, NULL, NULL, 0);
	return 0;
}

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

	GameMachine::instance().startGameMachine();
	return 0;
}