#include "stdafx.h"
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <regex>
#include "foundation/gamemachine.h"
#include <gmmessages.h>

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

		expandSource(entry); // 展开glsl
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

			GMStringReader reader(source);
			std::string report;
			GMint ln = 0;
			auto iter = reader.lineBegin();
			do
			{
				report += std::to_string(++ln) + ":\t" + (*iter).toStdString();
				iter++;
			} while (iter.hasNextLine());

			gm_error("Shader source: \n%s", report.c_str());
			gm_error("Shader compilation failed: %s", log);
			GM.postMessage({ GameMachineMessageType::Console, GMM_CONSOLE_SELECT_FILTER, GMM_CONSOLE_ERROR });
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
		GM_ASSERT("Shader linking failed: " && FALSE);
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

void GMGLShaderProgram::expandSource(REF GMGLShaderInfo& shaderInfo)
{
	// 解析源码，展开gm特有的宏
	shaderInfo.source = expandSource(shaderInfo.filename, shaderInfo.source);
}

GMString GMGLShaderProgram::expandSource(const GMString& filename, const GMString& source)
{
	GMStringReader reader(source);
	GMString n(""), line("");
	GMString expanded("");
	auto iter = reader.lineBegin();
	while (true)
	{
		line = *iter;
		if (matchMarco(line, "include", n))
			expandInclude(filename, n, line);
		expanded += line;
		if (!iter.hasNextLine())
			break;
		++iter;
	}
	return expanded;
}

bool GMGLShaderProgram::matchMarco(const GMString& source, const GMString& marco, REF GMString& result)
{
	std::string s = source.toStdString();
	std::string expr = "#(\\s*)" + marco.toStdString() + " \"(.*)\"";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		GM_ASSERT(match.size() >= 3);
		result = match[2].str();
		return true;
	}
	return false;
}

void GMGLShaderProgram::expandInclude(const GMString& filename, const GMString& fn, IN OUT GMString& source)
{
	GMString workingDir = GMPath::directoryName(filename);
	GMString include = workingDir + fn;
	GMBuffer buf;
	if (GM.getGamePackageManager()->readFileFromPath(include, &buf))
	{
		buf.convertToStringBuffer();
		source = expandSource(include, GMString((char*)buf.buffer));
	}
	else
	{
		gm_warning("GL shader '%s' not found, use empty file instead!", include);
	}
}