#include "stdafx.h"
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"

GLuint GMGLShaderProgram::Data::lastUsedProgram = -1;

GMGLShaderProgram::~GMGLShaderProgram()
{
	D(d);
	glDeleteProgram(d->shaderProgram);
}

void GMGLShaderProgram::useProgram()
{
	D(d);
	if (d->lastUsedProgram == d->shaderProgram)
		return;

	glUseProgram(d->shaderProgram);
	d->lastUsedProgram = d->shaderProgram;
	GM_CHECK_GL_ERROR();
}

void GMGLShaderProgram::attachShader(const GMGLShaderInfo& shaderCfgs)
{
	D(d);
	d->shaderInfos.push_back(shaderCfgs);
}

void GMGLShaderProgram::setMatrix4(const char* name, const GMfloat value[16])
{
	glUniformMatrix4fv(glGetUniformLocation(getProgram(), name), 1, GL_FALSE, value);
}

void GMGLShaderProgram::setVec4(const char* name, const GMfloat value[4])
{
	glUniform4fv(glGetUniformLocation(getProgram(), name), 1, value);
}

void GMGLShaderProgram::setVec3(const char* name, const GMfloat value[3])
{
	glUniform3fv(glGetUniformLocation(getProgram(), name), 1, value);
}

void GMGLShaderProgram::setInt(const char* name, GMint value)
{
	glUniform1i(glGetUniformLocation(getProgram(), name), value);
}

void GMGLShaderProgram::setFloat(const char* name, GMfloat value)
{
	glUniform1f(glGetUniformLocation(getProgram(), name), value);
}

void GMGLShaderProgram::setBool(const char* name, bool value)
{
	setInt(name, (GMint)value);
}

void GMGLShaderProgram::setMatrix4(const GMString& name, const GMfloat value[16])
{
	std::string stdName = name.toStdString();
	glUniformMatrix4fv(glGetUniformLocation(getProgram(), stdName.c_str()), 1, GL_FALSE, value);
}

void GMGLShaderProgram::setVec4(const GMString& name, const GMfloat value[4])
{
	std::string stdName = name.toStdString();
	glUniform4fv(glGetUniformLocation(getProgram(), stdName.c_str()), 1, value);
}

void GMGLShaderProgram::setVec3(const GMString& name, const GMfloat value[3])
{
	std::string stdName = name.toStdString();
	glUniform3fv(glGetUniformLocation(getProgram(), stdName.c_str()), 1, value);
}

void GMGLShaderProgram::setInt(const GMString& name, GMint value)
{
	std::string stdName = name.toStdString();
	glUniform1i(glGetUniformLocation(getProgram(), stdName.c_str()), value);
}

void GMGLShaderProgram::setFloat(const GMString& name, GMfloat value)
{
	std::string stdName = name.toStdString();
	glUniform1f(glGetUniformLocation(getProgram(), stdName.c_str()), value);
}

void GMGLShaderProgram::setBool(const GMString& name, bool value)
{
	setInt(name, (GMint)value);
}

void GMGLShaderProgram::load()
{
	D(d);
	if (d->shaderInfos.size() == 0)
		return;

	GLuint program = glCreateProgram();
	setProgram(program);

	for (auto& entry : d->shaderInfos)
	{
		GLuint shader = glCreateShader(entry.type);
		d->shaders.push_back(shader);

		std::string src = entry.source.toStdString();
		const GLchar* source = src.c_str();
		if (!source)
		{
			removeShaders();
			return;
		}

		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLsizei len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shader, len, &len, log);
			gm_error("Shader compilation failed: %s", log);
			ASSERT("Shader compilation failed: " && FALSE);
			delete[] log;
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

		removeShaders();
	}
}

void GMGLShaderProgram::removeShaders()
{
	D(d);
	GMGLShaderIDList& shadersInfo = d->shaders;
	for (auto& shader : shadersInfo)
	{
		glDeleteShader(shader);
	}
}