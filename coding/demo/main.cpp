#define GLEW_STATIC
#define FREEGLUT_STATIC

#include <windows.h>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "gmdatacore/objreader/objreader.h"
#include "utilities/path.h"
#include "utilities/camera.h"
#include "utilities/assert.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "gmengine/flow/gameloop.h"
#include "gmengine/io/keyboard.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/elements/character.h"
#include "gmgl/gmglshaders.h"
#include "gmgl/gmglfunc.h"
#include "gmgl/gmgltexture.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgllight.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglobjectpainter.h"
#include "utilities/vmath.h"
#include "gmengine/elements/cubegameobject.h"
#include "gmengine/elements/convexhullgameobject.h"
#include "gmengine/elements/skygameobject.h"
#include "gmengine/elements/spheregameobject.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 0;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 5;

GMfloat fps = 60;
GameLoopSettings s = { fps };

GameWorld world;
Character* character;

GLuint VAOs[1], Buffers[1], EBOs[1];

GLint render_model_matrix_loc;

class GameHandler : public IGameHandler
{
public:
	void setGameLoop(GameLoop* l)
	{
		m_gl = l;
	}

	void render()
	{
		Camera& camera = world.getMajorCharacter()->getCamera();
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world.getGraphicEngine());
		GMGLShaders& shaders = engine->getShaders();

		world.renderGameWorld();

		glutSwapBuffers();
	}

	void mouse()
	{
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world.getGraphicEngine());
		GMGLShaders& shaders = engine->getShaders();
		Camera& camera = world.getMajorCharacter()->getCamera();
		GMGL::lookAt(camera, shaders, GMSHADER_VIEW_MATRIX);
		int wx = glutGet(GLUT_WINDOW_X),
			wy = glutGet(GLUT_WINDOW_Y);
		camera.mouseReact(wx, wy, width, height);
	}

	void keyboard()
	{
		Character* character = world.getMajorCharacter();
		Camera* camera = &character->getCamera();
		GMfloat dis = 25;
		GMfloat v = dis / fps;
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
		{
			m_gl->terminate();
		}
		if (Keyboard::isKeyDown('A'))
			character->moveRight(-v);
		if (Keyboard::isKeyDown('D'))
			character->moveRight(v);
		if (Keyboard::isKeyDown('W'))
			character->moveFront(v);
		if (Keyboard::isKeyDown('S'))
			character->moveFront(-v);
		if (Keyboard::isKeyDown(VK_SPACE))
			character->jump();
	}

	void logicalFrame(GMfloat elapsed)
	{
		world.simulateGameWorld(elapsed);
	}

	GameLoop* m_gl;
};

GameHandler handler;
GameLoop* gl = GameLoop::getInstance();

void init()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_SMOOTH);

	std::string currentPath("D:\\shaders\\test\\");//Path::getCurrentPath();

	world.initialize();

	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(world.getGraphicEngine());
	GMGLShaders& shaders = engine->getShaders();
	ILightController& lightCtrl = engine->getLightController();

	GMGLShadowMapping& shadow = engine->getShadowMapping();
	GMGLShaders& shadowShaders = shadow.getShaders();

	{
		std::string vert = std::string(currentPath).append("gmshader.vert"),
			frag = std::string(currentPath).append("gmshader.frag");
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
		std::string vert = std::string(currentPath).append("gmshadowmapping.vert"),
			frag = std::string(currentPath).append("gmshadowmapping.frag");
		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, vert.c_str() },
			{ GL_FRAGMENT_SHADER, frag.c_str() },
		};
		shadowShaders.appendShader(shadersInfo[0]);
		shadowShaders.appendShader(shadersInfo[1]);
		shadowShaders.load();
	}

	world.setGravity(0, -100, 0);

	{
		btTransform charTransform;
		charTransform.setIdentity();
		charTransform.setOrigin(btVector3(0, 0, 100));
		character = new Character(charTransform, 10, 10, 15);
		character->setCanFreeMove(false);
		character->setJumpSpeed(btVector3(0, 50, 0));
		world.setMajorCharacter(character);
	}

	{
		btTransform groundTrans;
		groundTrans.setIdentity();
		groundTrans.setOrigin(btVector3(0, -100, 100));
		
		Material m [6] = {
			{ { .1f, .2f, .3f },{ .4f, .5f, .6f },{ .7f, .8f, .9f },{ 0, 0, 0 },1 },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ 0, 0, 0 }, 1 },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ 0, 0, 0 }, 1 },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ 0, 0, 0 }, 1 },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ 0, 0, 0 }, 1 },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ 0, 0, 0 }, 1 },
		};
		CubeGameObject* ground = new CubeGameObject(btVector3(300, 30, 500), groundTrans, m);
		ground->setMass(0);
		ground->getObject()->setPainter(new GMGLObjectPainter(shaders, shadow, ground->getObject()));
		ground->getObject()->getPainter()->init();
		world.appendObject(ground);
	}

	{
		Image* tex;	
		ImageReader::load("D:\\test.bmp", &tex);
		GMGLTexture* texture = new GMGLTexture(tex);

		btTransform boxTrans;
		boxTrans.setIdentity();
		boxTrans.setOrigin(btVector3(0, 0, 40));
		Material m[6] = {
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f }, { .8f, .8f, .8f }, 1, { texture, TextureTypeAmbient} },
			{ { .2f, .3f, .7f },{ .4f, .1f, .8f },{ .7f, .5f, .3f },{ .8f, .8f, .8f },  1,{ texture, TextureTypeAmbient } },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ .8f, .8f, .8f },  1,{ texture, TextureTypeAmbient } },
			{ { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },{ .8f, .8f, .8f },  1,{ texture, TextureTypeAmbient } },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ .8f, .8f, .8f },  1,{ texture, TextureTypeAmbient } },
			{ { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ .8f, .8f, .8f },  1,{ texture, TextureTypeAmbient } },
		};
		CubeGameObject* cube = new CubeGameObject(btVector3(10, 10, 10), boxTrans, m);
		cube->setMass(20);
		cube->getObject()->setPainter(new GMGLObjectPainter(shaders, shadow, cube->getObject()));
		world.appendObject(cube);
	}

	{
		ObjReader r;
		Object* obj;
		std::string objPath(std::string(currentPath).append("baymax.obj"));
		r.load("D:\\baymax.obj", &obj);
		for (auto iter = obj->getComponents().begin(); iter != obj->getComponents().end(); iter++)
		{
			(*iter)->getMaterial().Ke[0] = .5f;
			(*iter)->getMaterial().Ke[1] = .5f;
			(*iter)->getMaterial().Ke[2] = .5f;
		}

		btTransform boxTrans;
		boxTrans.setIdentity();
		boxTrans.setOrigin(btVector3(30, 70, 40));
		
		ConvexHullGameObject* convex = new ConvexHullGameObject(obj);
		convex->setLocalScaling(btVector3(0.5f, 0.5f, 0.5f));
		convex->setMass(10);
		convex->setTransform(boxTrans);
		convex->getObject()->setPainter(new GMGLObjectPainter(shaders, shadow, convex->getObject()));
		world.appendObject(convex);
	}

	{
		Image* tex;
		ImageReader::load("D:\\env.dds", &tex);
		GMGLTexture* texture = new GMGLTexture(tex);
		SkyGameObject* sky = new SkyGameObject(1000, texture);
		sky->getObject()->setPainter(new GMGLObjectPainter(shaders, shadow, sky->getObject()));
		world.setSky(sky);
	}

	{
		//Image* tex;
		//ImageReader::load("D:\\test.bmp", &tex);
		//GMGLTexture* texture = new GMGLTexture(tex);

		btTransform sphereTrans;
		sphereTrans.setIdentity();
		sphereTrans.setOrigin(btVector3(0, 40, 40));
		Material m = {
			 { .5f, .5f, .25f },{ .66f, .25f, .4f },{ .3f, .8f, .76f },{ .5, .5, .5 }, 1
		};
		SphereGameObject* sphere = new SphereGameObject(20, 30, 30, sphereTrans, m);
		sphere->setMass(20);
		sphere->getObject()->setPainter(new GMGLObjectPainter(shaders, shadow, sphere->getObject()));
		world.appendObject(sphere);
	}

	glEnable(GL_LINE_SMOOTH);

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);

	Camera& camera = world.getMajorCharacter()->getCamera();
	camera.mouseInitReaction(wx, wy, width, height);

	handler.setGameLoop(gl);
	camera.setSensibility(.25f);
	gl->init(s, &handler);

	GMfloat ambient[3] = { .5, .5, .5 };
	lightCtrl.setAmbient(ambient);

	GMfloat pos[3] = { 300,300,300 };
	lightCtrl.setLightPosition(pos);
	lightCtrl.setLightColor(ambient);
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
