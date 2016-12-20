#include "stdafx.h"
#include "objectdrawer.h"

void GMGLObjectDrawer::init(Object* obj)
{
	GLuint vao[1];
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);
	obj->setArrayId(vao[0]);

	GLuint vbo[1];
	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GMfloat) * obj->vao().size, obj->vao().data, GL_STATIC_DRAW);
	obj->setBufferId(vbo[0]);

	GLuint ebo[1];
	glGenBuffers(1, &ebo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GMuint) * obj->ebo().size, obj->ebo().data, GL_STATIC_DRAW);
	obj->setElementBufferId(ebo[0]);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void GMGLObjectDrawer::draw(Object* obj)
{
	glBindVertexArray(obj->getArrayId());

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(RESTART_INDEX);
	glDrawElements(GL_TRIANGLE_FAN, obj->ebo().size - 1, GL_UNSIGNED_INT, NULL);
	glDisable(GL_PRIMITIVE_RESTART);
}

void GMGLObjectDrawer::dispose(Object* obj)
{
	GLuint vao[1] = { obj->getArrayId() },
		vbo[1] = { obj->getBufferId() },
		ebo[1] = { obj->getElementBufferId() };
	glDeleteVertexArrays(1, vao);
	glDeleteBuffers(1, vbo);
	glDeleteBuffers(1, ebo);
}
