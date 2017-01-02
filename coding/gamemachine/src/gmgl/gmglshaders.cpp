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

static const GLchar* readShader(const char* filename)
{
#ifdef WIN32
	FILE* infile;
	fopen_s(&infile, filename, "rb");
#else
	FILE* infile = fopen(filename, "rb");
#endif // WIN32

	if (!infile) {
		ASSERT(false);
		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	GLchar* source = new GLchar[len + 1];

	fread(source, 1, len, infile);
	fclose(infile);

	source[len] = 0;

	return const_cast<const GLchar*>(source);
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

		const GLchar* source = readShader(entry->filename);
		if (source == NULL)
		{
			removeShaders(*this);
			return;
		}

		glShaderSource(shader, 1, &source, NULL);
		delete[] source;

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
