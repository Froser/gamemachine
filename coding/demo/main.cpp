#define GLEW_STATIC
#define FREEGLUT_STATIC

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

using namespace gm;

float width = 600;
float height = 300;

GameWorld* world;
Character* character;
GMGLFactory factory;

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
	void setGameLoop(GameLoop* l)
	{
		m_gl = l;
	}

	void render()
	{
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world->getGraphicEngine());
		GMGLShaders& shaders = engine->getShaders();

		world->renderGameWorld();

		glutSwapBuffers();
	}

	void mouse()
	{
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world->getGraphicEngine());
		GMGLShaders& shaders = engine->getShaders();
		CameraLookAt lookAt;
		Camera::calcCameraLookAt(world->getMajorCharacter()->getPositionState(), &lookAt);
		GMGL::lookAt(lookAt, shaders, GMSHADER_VIEW_MATRIX);
		int wx = glutGet(GLUT_WINDOW_X),
			wy = glutGet(GLUT_WINDOW_Y);
		reaction->mouseReact(wx, wy, width, height);
	}

	void keyboard()
	{
		Character* character = world->getMajorCharacter();
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
		{
			m_gl->terminate();
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

	GameLoop* m_gl;
};

GameHandler handler;
GraphicSettings settings = { 60 };
GameLoop* gl = GameLoop::getInstance();

void init()
{
	glEnable(GL_POLYGON_SMOOTH);

	GMMap* map;
#if _DEBUG
	GMMapReader::readGMM("D:/gmm/demo.xml", &map);
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

	GameWorldCreator::createGameWorld(&factory, map, &world);
	delete map;

	MouseReactionHandler* mouseHandler = new MouseReactionHandler(world);
	reaction = new MouseReaction(mouseHandler);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world->getGraphicEngine());
	engine->setGraphicSettings(&settings);

	GMGLShaders& shaders = engine->getShaders();

	GMGLShadowMapping& shadow = engine->getShadowMapping();
	GMGLShaders& shadowShaders = shadow.getShaders();

	{
		std::string vert = std::string(shaderPath).append("gmshader.vert"),
			frag = std::string(shaderPath).append("gmshader.frag");
		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, vert.c_str() },
			{ GL_FRAGMENT_SHADER, frag.c_str() },
		};
		shaders.appendShader(shadersInfo[0]);
		shaders.appendShader(shadersInfo[1]);
		shaders.load();
		shaders.useProgram();
	}

	{
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

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);

	reaction->initReaction(wx, wy, width, height);
	handler.setGameLoop(gl);
	gl->init(settings, &handler);
}

void render()
{
}

void resharp(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

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
	int argc = 1;
	char* l = "";
	char* argv[1];
	argv[0] = l;
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(400, 400);
	glutCreateWindow("Render");

	GLenum err = glewInit();
	init();
	glutReshapeFunc(resharp);
	glutDisplayFunc(render);

	gl->start();

	glutMainLoop();

	return 0;
}
