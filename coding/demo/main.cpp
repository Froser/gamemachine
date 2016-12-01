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

class SimpleGameObject : public GameObject
{
public:
	SimpleGameObject(const btVector3& vec, const btTransform& trans)
		: m_vec(vec)
		, m_trans(trans)
	{
	}

	virtual btMotionState* createMotionState() override
	{
		return new btDefaultMotionState(m_trans);
	}

	virtual void drawObject() override
	{
		glutSolidCube(50);
	}

	virtual btCollisionShape* createCollisionShape() override
	{
		return new btBoxShape(m_vec);
	}

	btVector3 m_vec;
	btTransform m_trans;
};

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
		gluPerspective(30, 2, 10, 300);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		CameraUtility::fglextlib_gl_LookAt(camera);

		//glColor3f(1, 1, 1);
		//obj.draw();

		world.renderGameWorld(m_gl->getSettings().fps);

		glEnd();

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

	GameLoop* m_gl;
};

GameHandler handler;
GameLoop gl(s, &handler);

void init()
{
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -0, 0));
	SimpleGameObject* gameObj = new SimpleGameObject(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)), groundTransform);
	gameObj->setMass(5);
	world.appendObject(gameObj);

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
	//glEnable(GL_LIGHTING);

	GLfloat pos[] = { 150, 150, 150, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	GLfloat clr[] = { .5, .5, .5, 1 };
	GLfloat b[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, clr);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, clr);
	glLightfv(GL_LIGHT0, GL_SPECULAR, b);
	//glEnable(GL_LIGHT0);
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
