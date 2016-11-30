#define GLEW_STATIC
#include <windows.h>
#include "GL/glew.h"
#include "glut.h"
#include "objreader/objreader.h"
#include "utilities/path.h"
#include "utilities/camera.h"
#include "utilities/assert.h"
#include "imagereader/imagereader.h"
#include "gameloop/gameloop.h"
#include "io/keyboard.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 149;
GLfloat eyeX = 0, eyeY = 750, eyeZ = 500;

ObjReader reader(ObjReader::LoadOnly);
Camera camera;

GMfloat fps = 60;
GameLoopSettings s = { fps };

GLuint tex;

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
		gluPerspective(30, 2, 10, 1500);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		CameraUtility::fglextlib_gl_LookAt(camera);

		glColor3f(1, 1, 1);
		reader.draw();

		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
// 		glBindTexture(GL_TEXTURE_2D, tex);
// 
// 		glBegin(GL_POLYGON);
// 		glTexCoord2f(0, 0); glVertex3d(0, 0, 0);
// 		glTexCoord2f(0, 1); glVertex3d(0, 30, 0);
// 		glTexCoord2f(1, 1); glVertex3d(30, 30, 0);
// 		glTexCoord2f(1, 0); glVertex3d(30, 0, 0);
// 		glEnd();

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
// 	ImageReader r;
// 	Image img;
// 	r.load("D:\\test.bmp", &img);
// 
// 	FByte* buf = img.asTexture();
// 	glGenTextures(1, &tex);
// 	glBindTexture(GL_TEXTURE_2D, tex);
// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buf);

	glEnable(GL_LINE_SMOOTH);

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);
	camera.mouseInitReaction(wx, wy, width, height);
	handler.setGameLoop(&gl);

	std::string path = Path::getCurrentPath();
	reader.load(path.append("newgirl.obj").c_str());
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
