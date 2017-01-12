#include "stdafx.h"
#include "gmglwindow.h"
#include "GL\freeglut_std.h"
#include "utilities\assert.h"

static void noop() {}

GMGLWindow::GMGLWindow(char* cmdLine, char* windowTitle, bool fullScreen)
	: m_fullscreen(fullScreen)
{
	int argc = 1;
	char* argv[1];
	argv[0] = cmdLine;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	strcpy(m_windowTitle, windowTitle);
}

void GMGLWindow::initWindowSize(GMfloat width, GMfloat height)
{
	if (!m_fullscreen)
	{
		glutInitWindowSize(width, height);
	}
	else
	{
		ASSERT(false);
	}
}

void GMGLWindow::setWindowResolution(GMfloat width, GMfloat height)
{
	if (!m_fullscreen)
	{
		glViewport(0, 0, width, height);
	}
	else
	{
		ASSERT(false);
	}
}

void GMGLWindow::setFullscreen(bool fullscreen)
{
	m_fullscreen = fullscreen;
	ASSERT(false);
}

void GMGLWindow::setWindowPosition(GMuint x, GMuint y)
{
	glutInitWindowPosition(x, y);
}

void GMGLWindow::createWindow()
{
	glutCreateWindow(m_windowTitle);
	glutDisplayFunc(noop);

	GLenum err;
	err = glewInit();
	ASSERT(err == GLEW_OK);
}

void GMGLWindow::startWindowLoop()
{
	glutMainLoop();
}

GMRect GMGLWindow::getWindowRect()
{
	GMRect bound = {
		glutGet(GLUT_WINDOW_X),
		glutGet(GLUT_WINDOW_Y),
		glutGet(GLUT_WINDOW_WIDTH),
		glutGet(GLUT_WINDOW_HEIGHT),
	};
	return bound;
}