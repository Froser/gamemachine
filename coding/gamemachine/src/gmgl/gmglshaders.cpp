#include "stdafx.h"
#include <stdio.h>
#include "utilities/assert.h"
#include "gmglshaders.h"

static void removeShaders(GMGLShaders& shaders)
{
	GMGLShadersInfo& shadersInfo = shaders.getShaders();
	for (auto iter = shadersInfo.begin(); iter != shadersInfo.end(); iter++)
	{
		GMGLShaderInfo* entry = &*iter;
		glDeleteShader(entry->shader);
		entry->shader = 0;
	}
}

GMGLShaders::~GMGLShaders()
{
	removeShaders(*this);
	glDeleteProgram(m_shaderProgram);
}

void GMGLShaders::useProgram()
{
	glUseProgram(m_shaderProgram);
	GLint i = glGetError();
	ASSERT_GL();
}

void GMGLShaders::appendShader(const GMGLShaderInfo& shader)
{
	m_shaders.push_back(shader);
}

void GMGLShaders::load()
{
	GMGLShadersInfo& shadersInfo = getShaders();
	if (shadersInfo.size() == 0)
		return;

	GLuint program = glCreateProgram();
	setProgram(program);

	for (auto iter = shadersInfo.begin(); iter != shadersInfo.end(); iter++)
	{
		GMGLShaderInfo* entry = &*iter;

		GLuint shader = glCreateShader(entry->type);
		entry->shader = shader;

		const GLchar* source = entry->data;
		if (source == NULL)
		{
			removeShaders(*this);
			return;
		}

		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
#ifdef _DEBUG
			GLsizei len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shader, len, &len, log);
			printf("%s", log);
			ASSERT("Shader compilation failed: " && FALSE);
			delete[] log;
#endif /* DEBUG */
			return;
		}

		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
#ifdef _DEBUG
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		printf("%s", log);
		ASSERT("Shader linking failed: " && FALSE);
		delete[] log;
#endif /* DEBUG */

		removeShaders(*this);
	}
}
