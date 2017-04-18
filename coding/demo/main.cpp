#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _WINDOWS

#include <windows.h>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/character.h"
#include "gmgl/gmglfactory.h"
#include "gmengine/controllers/gameloop.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglfunc.h"
#include "gmgl/shader_constants.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmgl/gmglwindow.h"
#include "utilities/path.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmdatacore/gameworldcreator.h"
#include "utilities/debug.h"
#include "utilities/input.h"

using namespace gm;

BSPGameWorld* world;
Character* character;
GMGLFactory factory;
GameMachine* gameMachine;

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
		m_input.initMouse(m_gm->getWindow());

#if _DEBUG
		std::string currentPath("D:/shaders/test/");
		std::string shaderPath("D:/shaders/test/");
		GameWorldCreator::createBSPGameWorld(m_gm, "D:/gv.bsp", &world);
#else
		std::string currentPath(Path::getCurrentPath());
		std::string shaderPath(currentPath);
		shaderPath.append("shaders/");
		std::string demoPath(currentPath);
		demoPath.append("gv.bsp");
		GameWorldCreator::createBSPGameWorld(m_gm, demoPath.c_str(), &world);
#endif
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(m_gm->getGraphicEngine());

		{
			GMGLShaders* shaders = new GMGLShaders();
			std::string vert = std::string(shaderPath).append("gmnormal.vert"),
				frag = std::string(shaderPath).append("gmnormal.frag");
			GMGLShaderInfo shadersInfo[] = {
				{ GL_VERTEX_SHADER, vert.c_str() },
				{ GL_FRAGMENT_SHADER, frag.c_str() },
			};
			shaders->appendShader(shadersInfo[0]);
			shaders->appendShader(shadersInfo[1]);
			shaders->load();
			shaders->useProgram();
			engine->registerShader(ChildObject::NormalObject, shaders);
		}

		{
			GMGLShaders* shaders = new GMGLShaders();
			std::string vert = std::string(shaderPath).append("gmsky.vert"),
				frag = std::string(shaderPath).append("gmsky.frag");
			GMGLShaderInfo shadersInfo[] = {
				{ GL_VERTEX_SHADER, vert.c_str() },
				{ GL_FRAGMENT_SHADER, frag.c_str() },
			};
			shaders->appendShader(shadersInfo[0]);
			shaders->appendShader(shadersInfo[1]);
			shaders->load();
			shaders->useProgram();
			engine->registerShader(ChildObject::Sky, shaders);
		}

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

	}

	void event(GameLoopEvent evt)
	{
		switch (evt)
		{
		case GAME_LOOP_RENDER:
			world->renderGameWorld();
			glutSwapBuffers();
			break;
		case GAME_LOOP_ACTIVATE_MESSAGE:
			static GMfloat mouseSensitivity = 0.25f;
			static GMfloat joystickSensitivity = 0.0003f;

			Character* character = world->getMajorCharacter();
			KeyboardState kbState = m_input.getKeyboardState();
			JoystickState joyState = m_input.getJoystickState();
			MouseState mouseState = m_input.getMouseState();

			if (kbState['Q'] || kbState[VK_ESCAPE])
				m_gm->getGameLoop()->terminate();

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

	void logicalFrame(GMfloat elapsed)
	{
		world->simulateGameWorld(elapsed);
	}

	void onExit()
	{
		delete world;
	}

	bool isWindowActivate()
	{
		GMGLWindow* window = static_cast<GMGLWindow*> (m_gm->getWindow());
		return GetActiveWindow() == window->getHWND();
	}

	GameMachine* getGameMachine()
	{
		return m_gm;
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
		new GMGLWindow(lpCmdLine, "GM", false ),
		new GMGLFactory(),
		new GameHandler()
	);

	gameMachine->startGameMachine();
	return 0;
}
