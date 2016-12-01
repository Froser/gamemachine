#define GLEW_STATIC
#include <windows.h>
#include "GL/glew.h"
#include "glut.h"
#include "objreader/objreader.h"
#include "core/objstruct.h"
#include "utilities/path.h"
#include "utilities/camera.h"
#include "utilities/assert.h"
#include "imagereader/imagereader.h"
#include "gameloop/gameloop.h"
#include "io/keyboard.h"
#include "gamescene/gameworld.h"
#include "gamescene/gameobject.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 0;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 300;

ObjReader reader(ObjReader::LoadOnly);
Camera camera;

GMfloat fps = 60;
GameLoopSettings s = { fps };

Object obj;
GameWorld world;

class GameHandler : public IGameHandler
{
public:
	void setGameLoop(GameLoop* l)
	{
		m_gl = l;
	}

	void render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(30, 2, 10, 3000);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		CameraUtility::fglextlib_gl_LookAt(camera);

		//glColor3f(1, 1, 1);
		//obj.draw();

		world.renderGameWorld();
		glEnable(GL_TEXTURE_2D);

		glutSwapBuffers();
	}

	void mouse()
	{
		int wx = glutGet(GLUT_WINDOW_X),
			wy = glutGet(GLUT_WINDOW_Y);
		camera.mouseReact(wx, wy, width, height);
	}

	void keyboard()
	{
		GMfloat dis = 50;
		GMfloat v = dis / fps;
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
			m_gl->terminate();
		if (Keyboard::isKeyDown('A'))
			camera.moveRight(-v);
		if (Keyboard::isKeyDown('D'))
			camera.moveRight(v);
		if (Keyboard::isKeyDown('W'))
			camera.moveFront(v);
		if (Keyboard::isKeyDown('S'))
			camera.moveFront(-v);
	}

	void logicalFrame()
	{
		world.simulateGameWorld(m_gl->getSettings().fps);
	}

	GameLoop* m_gl;
};

GameHandler handler;
GameLoop gl(s, &handler);

void init()
{
	Image img;
	ImageReader ir;
	ir.load("D:\\tests.bmp", &img);

	btTransform boxTransform;
	boxTransform.setIdentity();
	boxTransform.setOrigin(btVector3(0, 70, 0));
	
	GMfloat cubeClr[] = { 0.5f, 1.0f, 0.5f };
	GLCubeGameObject* gameObj = new GLCubeGameObject(
		25, 
		boxTransform,
		cubeClr);
	gameObj->setMass(5);
	world.appendObject(gameObj);

	btTransform boxTransform2;
	boxTransform.setIdentity();
	boxTransform.setOrigin(btVector3(20, 120, 10));

	GLCubeGameObject* gameObj3 = new GLCubeGameObject(
		25,
		boxTransform,
		cubeClr);
	gameObj3->setMass(10);
	world.appendObject(gameObj3);


	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -0, 0));

	GMfloat cubeClr2[] = { 1.f, 1.0f, 0.5f };
	GLCubeGameObject* ground = new GLCubeGameObject(
		50,
		groundTransform,
		cubeClr);
	ground->setMass(0);
	world.appendObject(ground);

	glEnable(GL_LINE_SMOOTH);

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);
	camera.mouseInitReaction(wx, wy, width, height);
	handler.setGameLoop(&gl);

	std::string path = Path::getCurrentPath();
	reader.load(path.append("fn57.obj").c_str(), &obj);

	camera.setSensibility(.25f);
	camera.setPosition(eyeX, eyeY, eyeZ);

	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	GLfloat pos[] = { 150, 150, 150, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	GLfloat clr[] = { .5, .5, .5, 1 };
	GLfloat b[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, clr);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, clr);
	glLightfv(GL_LIGHT0, GL_SPECULAR, b);
	glEnable(GL_LIGHT0);
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

	GameLoopUtilities::fglextlib_gl_registerGameLoop(gl);

	glutMainLoop();

	return 0;
}
