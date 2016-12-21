#include "stdafx.h"
#include "objectdrawer.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgl_func.h"

void GMGLObjectDrawer::init(Object* obj)
{
	GLuint vao[1];
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);
	obj->setArrayId(vao[0]);

	GLuint vaoSize = sizeof(GMfloat) * obj->vao().size,
		normalsSize = sizeof(GMfloat) * obj->normals().size;
	GLuint vbo[1];
	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vaoSize + normalsSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vaoSize, obj->vao().data);
	glBufferSubData(GL_ARRAY_BUFFER, vaoSize, normalsSize, obj->normals().data);
	obj->setBufferId(vbo[0]);

	GLuint ebo[1];
	glGenBuffers(1, &ebo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GMuint) * obj->ebo().size, obj->ebo().data, GL_STATIC_DRAW);
	obj->setElementBufferId(ebo[0]);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void GMGLObjectDrawer::draw(GMGLShaders& shaders, Object* obj)
{
	glBindVertexArray(obj->getArrayId());
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(RESTART_INDEX);

	for (auto iter = obj->getComponents().cbegin(); iter != obj->getComponents().cend(); iter++)
	{
		Component* component = (*iter);
		GMGL::ambient(component->getMaterial().Ka, shaders, GMSHADER_LIGHT_KA);
		glDrawElements(GL_LINE_LOOP, component->getCount() - 1, GL_UNSIGNED_INT, (void*) (sizeof(GMuint) * component->getOffset()));
	}
	glDisable(GL_PRIMITIVE_RESTART);
	glBindVertexArray(0);
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
