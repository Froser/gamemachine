#include <windows.h>
#include "glut.h"
#include "objreader/objreader.h"
#include "utilities/path.h"
#include "utilities/camera.h"

using namespace fglextlib;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 149;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 150;

ObjReader reader(ObjReader::LoadOnly);
Camera camera;

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	gluPerspective(30, 2, 10, 500);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	CameraUtility::lookAt(camera);

	glColor3f(1, 1, 1);
	reader.draw();

	glFlush();
}

void init()
{
	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);
	camera.mouseInitReaction(wx, wy, width, height);

	std::string path = Path::getCurrentPath();
	reader.load(path.append("cat.obj").c_str());
	camera.setSensibility(.5f);
	camera.setPosition(eyeX, eyeY, eyeZ);

	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	GLfloat pos[] = { 150, 150, 150, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	GLfloat clr[] = { 1, 1, 1, 1 };
	GLfloat b[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, clr);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, clr);
	glLightfv(GL_LIGHT0, GL_SPECULAR, b);
	glEnable(GL_LIGHT0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		camera.moveRight(-.75f);
		render();
		break;
	case 'd':
		camera.moveRight(.75f);
		render();
		break;
	case 's':
		camera.moveFront(-1);
		render();
		break;
	case 'w':
		camera.moveFront(1);
		render();
		break;
	}
}
void resharp(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void motion(int x, int y)
{
	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);
	camera.mouseReact(wx, wy, width, height);
	render();
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
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(400, 400);
	glutCreateWindow("Render");

	init();
	glutPassiveMotionFunc(motion);
	glutReshapeFunc(resharp);
	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
