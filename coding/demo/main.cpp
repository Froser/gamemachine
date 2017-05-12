#define GLEW_STATIC
#define FREEGLUT_STATIC
#define UNICODE
#include <windows.h>
#include "GL/glew.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/character.h"
#include "gmgl/gmglfactory.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglfunc.h"
#include "gmgl/shader_constants.h"
#include "gmengine/controllers/gamemachine.h"
#include "utilities/path.h"
#include "gmengine/elements/bspgameworld.h"
#include "utilities/debug.h"
#include "utilities/input.h"
#include "gmdatacore/gamepackage.h"

#include <fstream>
#include "gmengine/elements/glyphobject.h"
#include "os/wingl_window.h"
#include "gmdatacore/soundreader/soundreader.h"
#include "os/directsound_soundplayer.h"

using namespace gm;

BSPGameWorld* world;
Character* character;
GMGLFactory factory;
GameMachine* gameMachine;
GlyphObject* glyph;

// 这是一个导出所有资源的钩子，用gm_install_hook(GamePackage, readFileFromPath, resOutputHook)绑定此钩子
// 可以将所有场景中的资源导出到指定目录
static void resOutputHook(void* path, void* buffer)
{
	const char* resPath = (const char*)path;
	GamePackageBuffer* buf = (GamePackageBuffer*)buffer;
	if (buf->size == 0)
		return;

	std::fstream out;
	std::string p = std::string("D:/output/") + resPath;
	std::string dir = Path::directoryName(p);
	dir = dir.substr(0, dir.size() - 1);
	Path::createDirectory(dir);

	out.open(p, std::ios::out | std::ios::trunc | std::ios::binary);
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

	void setGameMachine(GameMachine* gm)
	{
		m_gm = gm;
	}

	void init()
	{
		//gm_install_hook(GamePackage, readFileFromPath, resOutputHook);
		m_input.initMouse(m_gm->getWindow());
		GamePackage pk(m_gm, &factory);
#ifdef _DEBUG
		pk.loadPackage("D:/gmpk");
#else
		pk.loadPackage((Path::getCurrentPath() + "gm.pk0").c_str());
#endif
		pk.createBSPGameWorld("gv.bsp", &world);

		glyph = new GlyphObject();
		glyph->setGeometry(-1, .8f, 1, 1);
		world->appendObjectAndInit(glyph, true);

		//GamePackageBuffer buf;
		//pk.readFileFromPath("D://test.wav", &buf);
		//ISoundFile* sf;
		//SoundReader::load(buf, &sf);
		//m_gm->getSoundPlayer()->play(sf, PlayOptions());

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
				GMWChar x[32], y[32], z[32], fps[32];
				swprintf_s(x, L"%f", position.position[0]);
				swprintf_s(y, L"%f", position.position[1]);
				swprintf_s(z, L"%f", position.position[2]);
				swprintf_s(fps, L"%f", gameMachine->getFPS());
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
			static GMfloat mouseSensitivity = 0.25f;
			static GMfloat joystickSensitivity = 0.0003f;

			Character* character = world->getMajorCharacter();
			KeyboardState kbState = m_input.getKeyboardState();
			JoystickState joyState = m_input.getJoystickState();
			MouseState mouseState = m_input.getMouseState();

			if (kbState['Q'] || kbState[VK_ESCAPE])
				m_gm->postMessage(GM_MESSAGE_EXIT);

			MoveAction moveTag = 0;
			MoveRate rate;

			if (kbState['A'])
				moveTag |= MD_LEFT;
			if (joyState.thumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_LEFT;
				rate.setMoveRate(MD_LEFT, GMfloat(joyState.thumbLX) / std::numeric_limits<decltype(joyState.thumbLX)>::min());
			}

			if (kbState['D'])
				moveTag |= MD_RIGHT;
			if (joyState.thumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_RIGHT;
				rate.setMoveRate(MD_RIGHT, GMfloat(joyState.thumbLX) / std::numeric_limits<decltype(joyState.thumbLX)>::max());
			}

			if (kbState['S'])
				moveTag |= MD_BACKWARD;
			if (joyState.thumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_BACKWARD;
				rate.setMoveRate(MD_BACKWARD, GMfloat(joyState.thumbLY) / std::numeric_limits<decltype(joyState.thumbLY)>::min());
			}

			if (kbState['W'])
				moveTag |= MD_FORWARD;
			if (joyState.thumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				moveTag |= MD_FORWARD;
				rate.setMoveRate(MD_FORWARD, GMfloat(joyState.thumbLY) / std::numeric_limits<decltype(joyState.thumbLY)>::max());
			}

			if (kbState[VK_SPACE] || joyState.buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
				moveTag |= MD_JUMP;

			if (kbState['V'])
				m_input.joystickVibrate(30000, 30000);
			else if (kbState['C'])
				m_input.joystickVibrate(0, 0);

			if (kbState['N'])
				DBG_SET_INT(DRAW_NORMAL, (DBG_INT(DRAW_NORMAL) + 1) % DRAW_NORMAL_MAX);

			if (joyState.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || joyState.thumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat) joyState.thumbRX / (
					joyState.thumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					std::numeric_limits<decltype(joyState.thumbRX)>::min() :
					std::numeric_limits<decltype(joyState.thumbRX)>::max() );

				world->getMajorCharacter()->lookRight(joyState.thumbRX * joystickSensitivity * rate);
			}
			if (joyState.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || joyState.thumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				GMfloat rate = (GMfloat)joyState.thumbRY / (
					joyState.thumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ?
					std::numeric_limits<decltype(joyState.thumbRY)>::min() :
					std::numeric_limits<decltype(joyState.thumbRY)>::max() );

				world->getMajorCharacter()->lookUp(joyState.thumbRY * joystickSensitivity * rate);
			}

			world->getMajorCharacter()->lookUp(-mouseState.deltaY * mouseSensitivity);
			world->getMajorCharacter()->lookRight(mouseState.deltaX * mouseSensitivity);

			character->action(moveTag, rate);

			if (kbState['P'])
				DBG_SET_INT(CALCULATE_BSP_FACE, !DBG_INT(CALCULATE_BSP_FACE));
			if (kbState['L'])
				DBG_SET_INT(POLYGON_LINE_MODE, !DBG_INT(POLYGON_LINE_MODE));
			if (kbState['O'])
				DBG_SET_INT(DRAW_ONLY_SKY, !DBG_INT(DRAW_ONLY_SKY));
			break;
		}
	}

	bool isWindowActivate()
	{
		WinGLWindow* window = static_cast<WinGLWindow*> (m_gm->getWindow());
		return GetActiveWindow() == window->hwnd();
	}
	
	GameMachine* m_gm;
	Input m_input;
};

GraphicSettings settings = { 60, { 700, 400 } ,{ 100, 100 }, {400, 400}, false };

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
	gameMachine = new GameMachine(
		settings,
		new GMGLFactory(),
		new GameHandler()
	);

	gameMachine->startGameMachine();
	return 0;
}
