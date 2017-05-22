#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "common.h"
#include "utilities/vector.h"
BEGIN_NS

struct GMGLShaderInfo
{
	GLenum type;
	const char* data;
	GLuint shader;
};

typedef Vector<GMGLShaderInfo> GMGLShadersInfo;

class GMGLShaders
{
public:
	GMGLShaders() : m_shaderProgram(0) {}
	~GMGLShaders();

	void load();

	void useProgram();
	GLuint getProgram() { return m_shaderProgram; }

	void appendShader(const GMGLShaderInfo& shader);
	GMGLShadersInfo& getShaders() { return m_shaders; }
	void setProgram(GLuint program) { m_shaderProgram = program; }

private:
	GMGLShadersInfo m_shaders;
	GLuint m_shaderProgram;
};

END_NS
#endif