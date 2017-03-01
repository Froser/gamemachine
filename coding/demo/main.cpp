#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _WINDOWS

#include <windows.h>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/character.h"
#include "gmgl/gmglfactory.h"
#include "gmengine/io/mouse.h"
#include "gmengine/controllers/gameloop.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmengine/io/keyboard.h"
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

class MouseReactionHandler : public IMouseReactionHandler
{
public:
	MouseReactionHandler(GameWorld* world)
		: m_world(world)
	{

	}

public:
	virtual void onMouseMove(GMfloat deltaX, GMfloat deltaY)
	{
		world->getMajorCharacter()->lookRight(deltaX * .25);
		world->getMajorCharacter()->lookUp(-deltaY * .25);
	}

private:
	GameWorld* m_world;
};

MouseReaction* reaction;

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
		m_input.reset(new Input());

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


		MouseReactionHandler* mouseHandler = new MouseReactionHandler(world);
		reaction = new MouseReaction(mouseHandler);

		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
			GMGLShadowMapping& shadow = engine->getShadowMapping();
			GMGLShaders& shadowShaders = shadow.getShaders();
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

		glEnable(GL_LINE_SMOOTH);

		GMRect rect = m_gm->getWindow()->getWindowRect();
		reaction->initReaction(rect);
	}

	void render()
	{
		world->renderGameWorld();
		glutSwapBuffers();
	}

	void mouse()
	{
		GMRect rect = m_gm->getWindow()->getWindowRect();
		reaction->mouseReact(rect);
	}

	void keyboard()
	{
		Character* character = world->getMajorCharacter();
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
		{
			m_gm->getGameLoop()->terminate();
		}

		MoveAction moveTag = 0;
		if (Keyboard::isKeyDown('A'))
			moveTag |= MD_LEFT;
		if (Keyboard::isKeyDown('D'))
			moveTag |= MD_RIGHT;
		if (Keyboard::isKeyDown('W'))
			moveTag |= MD_FORWARD;
		if (Keyboard::isKeyDown('S'))
			moveTag |= MD_BACKWARD;
		if (Keyboard::isKeyDown(VK_SPACE))
			moveTag |= MD_JUMP;

		character->action(moveTag);

		if (Keyboard::isKeyDown('P'))
			DBG_SET_INT(CALCULATE_BSP_FACE, !DBG_INT(CALCULATE_BSP_FACE));
		if (Keyboard::isKeyDown('L'))
			DBG_SET_INT(POLYGON_LINE_MODE, !DBG_INT(POLYGON_LINE_MODE));
		if (Keyboard::isKeyDown('O'))
			DBG_SET_INT(DRAW_ONLY_SKY, !DBG_INT(DRAW_ONLY_SKY));
	}

	void logicalFrame(GMfloat elapsed)
	{
		world->simulateGameWorld(elapsed);
	}

	void onExit()
	{
		delete world;
		delete reaction;
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
	AutoPtr<Input> m_input;
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
