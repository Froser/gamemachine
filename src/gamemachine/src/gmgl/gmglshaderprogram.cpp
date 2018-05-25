#include "stdafx.h"
#include <GL/glew.h>
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <regex>
#include "foundation/gamemachine.h"
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
	
	glUseProgram(d->shaderProgram);
	d->lastUsedProgram = d->shaderProgram;
	
	GMGLGraphicEngine* engine = gm_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	engine->shaderProgramChanged(this);
}

void GMGLShaderProgram::attachShader(const GMGLShaderInfo& shaderCfgs)
{
	D(d);
	d->shaderInfos.push_back(shaderCfgs);
}

void GMGLShaderProgram::setMatrix4(const char* name, const GMMat4& value)
{
	GM_ASSERT(verify());
	glUniformMatrix4fv(glGetUniformLocation(getProgram(), name), 1, GL_FALSE, ValuePointer(value));
}

void GMGLShaderProgram::setVec4(const char* name, const GMFloat4& value)
{
	GM_ASSERT(verify());
	glUniform4fv(glGetUniformLocation(getProgram(), name), 1, ValuePointer(value));
}

void GMGLShaderProgram::setVec3(const char* name, const GMfloat value[3])
{
	GM_ASSERT(verify());
	glUniform3fv(glGetUniformLocation(getProgram(), name), 1, value);
}

void GMGLShaderProgram::setInt(const char* name, GMint value)
{
	GM_ASSERT(verify());
	glUniform1i(glGetUniformLocation(getProgram(), name), value);
}

void GMGLShaderProgram::setFloat(const char* name, GMfloat value)
{
	GM_ASSERT(verify());
	glUniform1f(glGetUniformLocation(getProgram(), name), value);
}

void GMGLShaderProgram::setBool(const char* name, bool value)
{
	setInt(name, (GMint)value);
}

bool GMGLShaderProgram::setInterfaceInstance(const char* interfaceName, const char* instanceName, GMShaderType type)
{
	GLenum shaderType = GL_VERTEX_SHADER;
	if (type == GMShaderType::Vertex)
		shaderType = GL_VERTEX_SHADER;
	else if (type == GMShaderType::Pixel)
		shaderType = GL_FRAGMENT_SHADER;
	else
		GM_ASSERT(false);
	
	return setSubrotinue(interfaceName, instanceName, shaderType);
}

bool GMGLShaderProgram::setSubrotinue(const char* funcName, const char* implement, GMuint shaderType)
{
	GM_ASSERT(verify());
	D(d);
	GLint subroutineUniform = glGetSubroutineUniformLocation(d->shaderProgram, shaderType, funcName);
	if (subroutineUniform >= 0)
	{
		GLuint subroutineIndex = glGetSubroutineIndex(d->shaderProgram, shaderType, implement);
		if (subroutineIndex != GL_INVALID_INDEX)
		{
			GLsizei n;
			glGetProgramStageiv(d->shaderProgram, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &n);
			GM_ASSERT(n == 1); //GM目前只用1个subroutine来决定渲染流程。subroutine这个特性真的是太难用了
			glUniformSubroutinesuiv(shaderType, n, &subroutineIndex);
			return true;
		}
	}
	return false;
}

bool GMGLShaderProgram::verify()
{
	D(d);
	return d->lastUsedProgram == d->shaderProgram;
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
		const std::string& src = entry.source.toStdString();
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
			GMMessage consoleMsg(GameMachineMessageType::Console, GMM_CONSOLE_SELECT_FILTER, GMM_CONSOLE_ERROR);
			GM.postMessage(consoleMsg);
			GMMessage crashMsg(GameMachineMessageType::CrashDown);
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
		GMMessage consoleMsg(GameMachineMessageType::Console, GMM_CONSOLE_SELECT_FILTER, GMM_CONSOLE_ERROR);
		GM.postMessage(consoleMsg);
		GMMessage crashMsg(GameMachineMessageType::CrashDown);
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