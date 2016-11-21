#include <windows.h>
#include "glut.h"
#include "objreader/objreader.h"

using namespace fglextlib;

float WINDOW_WIDTH = 600;
float WINDOW_HEIGHT = 300;

void render()
{
	ObjReader reader;
	reader.load("D:\\baymax.obj");
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
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(400, 400);
	glutCreateWindow("Render");

	glutDisplayFunc(render);
	glutMainLoop();

	return 0;
}
