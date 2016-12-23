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
#include "utilities/vmath.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 0;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 5;

GMfloat fps = 60;
GameLoopSettings s = { fps };

GameWorld world;
Character* character;

GMGLShaders shaders;

GLuint VAOs[1], Buffers[1], EBOs[1];

GLint render_model_matrix_loc;

GMGLTexture texture;

Object* obj;

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
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		Camera& camera = world.getMajorCharacter()->getCamera();

		GMGL::lookAt(camera, shaders, GMSHADER_VIEW_MATRIX);
		obj->getDrawer()->draw(shaders, obj);

		//glBindVertexArray(VAOs[0]);
		//glEnable(GL_PRIMITIVE_RESTART);
		//glPrimitiveRestartIndex(0xFFFF);
		//glDrawElements(GL_TRIANGLE_FAN, 9, GL_UNSIGNED_SHORT, NULL);
		//glDisable(GL_PRIMITIVE_RESTART);

		//glColor3f(1, 1, 1);
		//obj.draw();
		//world.renderGameWorld();

		glutSwapBuffers();
	}

	void mouse()
	{
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
			delete obj;
			m_gl->terminate();
		}
		if (Keyboard::isKeyDown('A'))
			camera->moveRight(-v);
		if (Keyboard::isKeyDown('D'))
			camera->moveRight(v);
		if (Keyboard::isKeyDown('W'))
			camera->moveFront(v);
		if (Keyboard::isKeyDown('S'))
			camera->moveFront(-v);
		//if (Keyboard::isKeyDown(VK_SPACE))
		//	camera->jump();
	}

	void logicalFrame()
	{
		//world.simulateGameWorld(m_gl->getSettings().fps);
	}

	GameLoop* m_gl;
};

GameHandler handler;
GameLoop gl(s, &handler);


void init()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	obj = nullptr;
	ObjReader r;
	r.load("D:\\baymax.obj", &obj);
	obj->getDrawer()->init(obj);

	world.setGravity(0, 0, 0);

	btTransform charTransform;
	charTransform.setIdentity();
	charTransform.setOrigin(btVector3(0, 100, 100));
	character = new Character(charTransform, 0, 10, 15);
	character->setCanFreeMove(true);
	character->setJumpSpeed(btVector3(0, 50, 0));
	world.setMajorCharacter(character);

	glEnable(GL_LINE_SMOOTH);

	int wx = glutGet(GLUT_WINDOW_X),
		wy = glutGet(GLUT_WINDOW_Y);

	Camera& camera = world.getMajorCharacter()->getCamera();
	camera.mouseInitReaction(wx, wy, width, height);

	handler.setGameLoop(&gl);
	camera.setSensibility(.25f);

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, "D:/shaders/test/test.vert" },
		{ GL_FRAGMENT_SHADER, "D:/shaders/test/test.frag" },
	};
	shaders.appendShader(shadersInfo[0]);
	shaders.appendShader(shadersInfo[1]);
	shaders.load();
	shaders.useProgram();

	render_model_matrix_loc = glGetUniformLocation(shaders.getProgram(), GMSHADER_MODEL_MATRIX);

	using namespace vmath;
	vmath::mat4 model_matrix(mat4::identity());
	glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
	GMGL::perspective(30, 2, 1, 1000, shaders, GMSHADER_PROJECTION_MATRIX);

	GMfloat ambient[3] = { .5, .5, .5 };
	GMGLLight(shaders).setAmbient(ambient);

	GMfloat pos[3] = { 100,100,100 };
	GMGLLight(shaders).setLightPosition(pos);
	GMGLLight(shaders).setLightColor(ambient);
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

	GameLoopUtilities::gm_gl_registerGameLoop(gl);

	glutMainLoop();

	return 0;
}
