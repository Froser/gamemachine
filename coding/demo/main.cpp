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
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "gmgl/shaders.h"
#include "utilities/vmath.h"
#include "gmgl/gmgl_func.h"
#include "gmgl/texture.h"

using namespace gm;

float width = 600;
float height = 300;
GLfloat centerX = 0, centerY = 0, centerZ = 0;
GLfloat eyeX = 0, eyeY = 0, eyeZ = 5;

ObjReader reader(ObjReader::LoadOnly);

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

		Camera& camera = world.getMajorCharacter()->getCamera();

		GMGL::lookAt(camera, shaders, "view_matrix");
		obj->getDrawer()->draw(obj);

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
		GMGL::lookAt(camera, shaders, "view_matrix");
		int wx = glutGet(GLUT_WINDOW_X),
			wy = glutGet(GLUT_WINDOW_Y);
		camera.mouseReact(wx, wy, width, height);
	}

	void keyboard()
	{
		Character* character = world.getMajorCharacter();
		Camera* camera = &character->getCamera();
		GMfloat dis = 5;
		GMfloat v = dis / fps;
		if (Keyboard::isKeyDown(VK_ESCAPE) || Keyboard::isKeyDown('Q'))
			m_gl->terminate();
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//GMGLTexture::loadTexture("D:\\curiosity.dds", texture);
	obj = nullptr;
	ObjReader r(ObjReader::LoadOnly);
	r.load("D:\\cat.obj", &obj);
	obj->getDrawer()->init(obj);

	world.setGravity(0, 0, 0);

	btTransform charTransform;
	charTransform.setIdentity();
	charTransform.setOrigin(btVector3(0, 10, 10));
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

	//VAO, VBO
	GMfloat pos[] = {
		1.0000,  1.0000,  1.0000,
		-1.0000,  1.0000,  1.0000,
		-1.0000, -1.0000,  1.0000,
		1.0000, -1.0000,  1.0000,
		1.0000,  1.0000, -1.0000,
		-1.0000,  1.0000, -1.0000,
		-1.0000, -1.0000, -1.0000,
		1.0000, -1.0000, -1.0000,
	};
	GLushort indices[] = {
		0, 1, 2, 3,
		0xFFFF,
		1, 5, 6, 2,
	};

	glGenVertexArrays(1, &VAOs[0]);
	glBindVertexArray(VAOs[0]);

	GLuint vbo[1];
	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	GLuint ebo[1];
	glGenBuffers(1, &ebo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, "D:/shaders/test/test.vert" },
		{ GL_FRAGMENT_SHADER, "D:/shaders/test/test.frag" },
	};
	shaders.appendShader(shadersInfo[0]);
	shaders.appendShader(shadersInfo[1]);
	shaders.load();
	shaders.useProgram();

	render_model_matrix_loc = glGetUniformLocation(shaders.getProgram(), "model_matrix");

	using namespace vmath;
	vmath::mat4 model_matrix(mat4::identity());
	glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
	GMGL::perspective(30, 2, 1, 1000, shaders, "projection_matrix");

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
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
