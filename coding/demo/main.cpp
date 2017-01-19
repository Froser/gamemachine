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
#include "gmengine/flow/gameloop.h"
#include "gmdatacore/gmmap/gmmap.h"
#include "gmdatacore/gmmap/gmmapreader.h"
#include "gmdatacore/gmmap/gameworldcreator.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmengine/io/keyboard.h"
#include "gmgl/gmglfunc.h"
#include "gmgl/shader_constants.h"
#include "gmengine/controller/gamemachine.h"
#include "gmgl/gmglwindow.h"
#include "utilities/path.h"

using namespace gm;

GameWorld* world;
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
		glEnable(GL_POLYGON_SMOOTH);

		GMMap* map;
#if _DEBUG
		GMMapReader::readGMM("D:/stage1/demo.xml", &map);
		std::string currentPath("D:/shaders/test/");
		std::string shaderPath("D:/shaders/test/");
#else
		std::string currentPath(Path::getCurrentPath());
		std::string demoPath(currentPath);
		demoPath.append("map/demo.xml");
		GMMapReader::readGMM(demoPath.c_str(), &map);
		std::string shaderPath(currentPath);
		shaderPath.append("shaders/");
#endif

		GameWorldCreator::createGameWorld(m_gm, map, &world);
		delete map;

		MouseReactionHandler* mouseHandler = new MouseReactionHandler(world);
		reaction = new MouseReaction(mouseHandler);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
		if (Keyboard::isKeyDown('A'))
			character->moveLeft();
		if (Keyboard::isKeyDown('D'))
			character->moveRight();
		if (Keyboard::isKeyDown('W'))
			character->moveForward();
		if (Keyboard::isKeyDown('S'))
			character->moveBackward();
		if (Keyboard::isKeyDown(VK_SPACE))
			character->jump();
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

	GameMachine* m_gm;
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
