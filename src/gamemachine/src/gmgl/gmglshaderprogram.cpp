#include "stdafx.h"
#include <GL/glew.h>
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <regex>
#include "foundation/gamemachine.h"
#include <gmmessages.h>
#include <linearmath.h>

GLuint GMGLShaderProgram::Data::lastUsedProgram = -1;

GMuint GMGLShaderInfo::toGLShaderType(GMShaderType type)
{
	switch (type)
	{
	case GMShaderType::Pixel:
		return GL_FRAGMENT_SHADER;
	case GMShaderType::Vertex:
		return GL_VERTEX_SHADER;
	default:
		GM_ASSERT(false);
		return GL_NONE;
	}
}

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

	GM_BEGIN_CHECK_GL_ERROR
	glUseProgram(d->shaderProgram);
	GM_END_CHECK_GL_ERROR
	d->lastUsedProgram = d->shaderProgram;
}

void GMGLShaderProgram::attachShader(const GMGLShaderInfo& shaderCfgs)
{
	D(d);
	d->shaderInfos.push_back(shaderCfgs);
}

void GMGLShaderProgram::setMatrix4(const char* name, const GMMat4& value)
{
	glUniformMatrix4fv(glGetUniformLocation(getProgram(), name), 1, GL_FALSE, ValuePointer(value));
}

void GMGLShaderProgram::setVec4(const char* name, const GMFloat4& value)
{
	glUniform4fv(glGetUniformLocation(getProgram(), name), 1, ValuePointer(value));
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

void GMGLShaderProgram::setMatrix4(const GMString& name, const GMMat4& value)
{
	std::string stdName = name.toStdString();
	glUniformMatrix4fv(glGetUniformLocation(getProgram(), stdName.c_str()), 1, GL_FALSE, ValuePointer(value));
}

void GMGLShaderProgram::setVec4(const GMString& name, const GMFloat4& value)
{
	std::string stdName = name.toStdString();
	glUniform4fv(glGetUniformLocation(getProgram(), stdName.c_str()), 1, ValuePointer(value));
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
			GameMachineMessage consoleMsg(GameMachineMessageType::Console, GMM_CONSOLE_SELECT_FILTER, GMM_CONSOLE_ERROR);
			GM.postMessage(consoleMsg);
			GameMachineMessage crashMsg(GameMachineMessageType::CrashDown);
			GM.postMessage(crashMsg);
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
		gm_error("%s", log);
		GameMachineMessage consoleMsg(GameMachineMessageType::Console, GMM_CONSOLE_SELECT_FILTER, GMM_CONSOLE_ERROR);
		GM.postMessage(consoleMsg);
		GameMachineMessage crashMsg(GameMachineMessageType::CrashDown);
		GM.postMessage(crashMsg);
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
	GMString n, line;
	GMString expanded;
	auto iter = reader.lineBegin();
	while (true)
	{
		line = *iter;
		if (matchMacro(line, "include", n))
			expandInclude(filename, n, line);
		else if (matchMacro(line, "alias", n))
			expandAlias(n, line);
		expanded += replaceLine(line);
		if (!iter.hasNextLine())
			break;
		++iter;
	}
	return expanded;
}

bool GMGLShaderProgram::matchMacro(const GMString& source, const GMString& macro, REF GMString& result)
{
	std::string s = source.toStdString();
	std::string expr = "#(\\s*)" + macro.toStdString() + "(\\s+)(.*)";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		GM_ASSERT(match.size() >= 4);
		result = match[3].str();
		return true;
	}
	return false;
}

void GMGLShaderProgram::expandInclude(const GMString& workingDir, const GMString& fn, IN OUT GMString& source)
{
	std::string f;
	std::string s = source.toStdString();
	static std::string expr = "(\\s*)\"(.+)\"";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		if (match.size() != 3)
			return;
		f = match[2].str();
	}

	GMString dir = GMPath::directoryName(workingDir);
	GMString include = dir + f;
	GMBuffer buf;
	if (GM.getGamePackageManager()->readFileFromPath(include, &buf))
	{
		buf.convertToStringBuffer();
		source = expandSource(include, GMString((char*)buf.buffer)) + GM_CRLF;
	}
	else
	{
		gm_warning("GL shader '%s' not found, use empty file instead!", include);
	}
}

void GMGLShaderProgram::expandAlias(const GMString& alias, IN OUT GMString& source)
{
	D(d);
	std::string s = alias.toStdString();
	static std::string expr = "(\\s*)(\\S+)(\\s+)(.+)";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		GM_ASSERT(match.size() == 5);
		std::string replacement = match[2].str();
		std::string a = match[4].str();
		d->aliasMap.insert({ "${" + replacement + "}", a });
	}
	source = L"";
}

GMString& GMGLShaderProgram::replaceLine(IN OUT GMString& line)
{
	D(d);
	for (auto& alias : d->aliasMap)
	{
		line = line.replace(alias.first, alias.second);
	}
	return line;
}

bool GMGLShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMGLShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}
