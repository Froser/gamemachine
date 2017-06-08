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
#include "os/gmwingl_window.h"
#include "gmui/gmui.h"

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

class GameHandler : public IGameHandler
{
public:
	GameHandler()
	{
	}

	void init()
	{
		//gm_install_hook(GMGamePackage, readFileFromPath, resOutputHook);
		GMInput* inputManager = GameMachine::instance().getInputManager();
		inputManager->initMouse(GameMachine::instance().getWindow());
		GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
#ifdef _DEBUG
		pk->loadPackage("D:/gmpk");
#else
		pk->loadPackage((Path::getCurrentPath() + "gm.pk0").c_str());
#endif
		pk->createBSPGameWorld("gv.bsp", &world);

		glyph = new GMGlyphObject();
		glyph->setGeometry(-1, .8f, 1, 1);
		world->appendObjectAndInit(glyph, true);

		//GMBuffer bg;
		//pk.readFile(PI_SOUNDS, "bgm/bgm.mp3", &bg);
		//SoundReader::load(bg, &sf);
		//sf->play();

		/*
		{
			GMGLShadowMapping* shadow = engine->getShadowMapping();
			GMGLShaders& shadowShaders = shadow->getShaders();
			std::string vert = std::string(shaderPath).append("gmshadowmapping.vert"),
				frag = std::string(shaderPath).append("gmshadowmapping.frag");
			GMGLShaderInfo shadersInfo[] = {
				{ GL_VERTEX_SHADER, vert.c_str() },
				{ GL_FRAGMENT_SHADER, frag.c_str() },
			};
			shadowShaders.appendShader(shadersInfo[0]);
			shadowShaders.appendShader(shadersInfo[1]);
			shadowShaders.load();
		}
		*/
	}

	void event(GameMachineEvent evt)
	{
		switch (evt)
		{
		case GM_EVENT_SIMULATE:
			world->simulateGameWorld();
			break;
		case GM_EVENT_RENDER:
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
		case GM_EVENT_ACTIVATE:
			GMInput* inputManager = GameMachine::instance().getInputManager();
			static GMfloat mouseSensitivity = 0.25f;
			static GMfloat joystickSensitivity = 0.0003f;

			GMCharacter* character = world->getMajorCharacter();
			GMKeyboardState kbState = inputManager->getKeyboardState();
			GMJoystickState joyState = inputManager->getJoystickState();
			GMMouseState mouseState = inputManager->getMouseState();

			if (kbState['Q'] || kbState[VK_ESCAPE])
				GameMachine::instance().postMessage(GM_MESSAGE_EXIT);

			MoveAction moveTag = 0;
			MoveRate rate;

			if (kbState['A'])
				moveTag |= MD_LEFT;
			if (joyState.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_LEFT;
				rate.setMoveRate(MD_LEFT, GMfloat(joyState.thumbLX) / SHRT_MIN);
			}

			if (kbState['D'])
				moveTag |= MD_RIGHT;
			if (joyState.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_RIGHT;
				rate.setMoveRate(MD_RIGHT, GMfloat(joyState.thumbLX) / SHRT_MAX);
			}

			if (kbState['S'])
				moveTag |= MD_BACKWARD;
			if (joyState.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_BACKWARD;
				rate.setMoveRate(MD_BACKWARD, GMfloat(joyState.thumbLY) / SHRT_MIN);
			}

			if (kbState['W'])
				moveTag |= MD_FORWARD;
			if (joyState.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_FORWARD;
				rate.setMoveRate(MD_FORWARD, GMfloat(joyState.thumbLY) / SHRT_MAX);
			}

			if (kbState[VK_SPACE] || joyState.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
				moveTag |= MD_JUMP;

			if (kbState['V'])
				inputManager->joystickVibrate(30000, 30000);
			else if (kbState['C'])
				inputManager->joystickVibrate(0, 0);

			if (kbState['N'])
				GMSetBuiltIn(DRAW_NORMAL, (GMGetBuiltIn(DRAW_NORMAL) + 1) % GMConfig_BuiltInOptions::DRAW_NORMAL_END);

			if (kbState['I'])
				GMSetBuiltIn(RUN_PROFILE, !GMGetBuiltIn(RUN_PROFILE));

			if (joyState.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || joyState.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat) joyState.thumbRX / (
					joyState.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				world->getMajorCharacter()->lookRight(joyState.thumbRX * joystickSensitivity * rate);
			}
			if (joyState.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || joyState.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat)joyState.thumbRY / (
					joyState.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					SHRT_MIN :
					SHRT_MAX);

				world->getMajorCharacter()->lookUp(joyState.thumbRY * joystickSensitivity * rate);
			}

			world->getMajorCharacter()->lookUp(-mouseState.deltaY * mouseSensitivity);
			world->getMajorCharacter()->lookRight(mouseState.deltaX * mouseSensitivity);

			character->action(moveTag, rate);

			if (kbState['P'])
				GMSetBuiltIn(CALCULATE_BSP_FACE, !GMGetBuiltIn(CALCULATE_BSP_FACE));
			if (kbState['L'])
				GMSetBuiltIn(POLYGON_LINE_MODE, !GMGetBuiltIn(POLYGON_LINE_MODE));
			if (kbState['O'])
				GMSetBuiltIn(DRAW_ONLY_SKY, !GMGetBuiltIn(DRAW_ONLY_SKY));
			break;
		}
	}

	bool isWindowActivate()
	{
		GMWinGLWindow* window = static_cast<GMWinGLWindow*> (GameMachine::instance().getWindow());
		return GetActiveWindow() == window->hwnd();
	}
};

GraphicSettings settings = { 60, { 700, 400 } ,{ 100, 100 }, {400, 400}, false };

int main()
{
	WinMain(NULL, NULL, NULL, 0);
	return 0;
}
/*
class Console : public GMUIWindow
{
public:
	Console(HINSTANCE h)
	{
		GMUIResourceManager::setResourceInstance(h);
	}

	virtual LPCTSTR getWindowClassName() const override
	{
		return L"Console";
	}

};
*/
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	char * lpCmdLine,
	int nCmdShow
)
{
	//HRESULT Hr = ::CoInitialize(NULL);
	//if (FAILED(Hr)) return 0;
	//
	//Console* pFrame = new Console(hInstance);
	//GMUIWindowAttributes attrs =
	//{
	//	NULL,
	//	L"",
	//	0,
	//	0,
	//	{ 0, 0, 1024 / 2, 738 / 2 }, 
	//	NULL,
	//};
	//
	//pFrame->create(attrs);
	//pFrame->centerWindow();
	//::ShowWindow(*pFrame, SW_SHOWMAXIMIZED);

	//CPaintManagerUI::MessageLoop();

	//::CoUninitialize();

	GameMachine::instance().init(
		settings,
		new GMGLFactory(),
		new GameHandler()
	);

	GameMachine::instance().startGameMachine();
	return 0;
}