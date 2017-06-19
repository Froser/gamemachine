#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "common.h"
#include "foundation/vector.h"
BEGIN_NS

struct GMGLShaderInfo
{
	GLenum type;
	GMString source;
};

typedef AlignedVector<GLuint> GMGLShaderIDList;
typedef AlignedVector<GMGLShaderInfo> GMGLShaderInfos;

GM_PRIVATE_OBJECT(GMGLShaderProgram)
{
	GMGLShaderInfos shaderInfos;
	GMGLShaderIDList shaders;
	GLuint shaderProgram = 0;
};

class GMGLShaderProgram : public GMObject
{
	DECLARE_PRIVATE(GMGLShaderProgram);

public:
	GMGLShaderProgram() = default;
	~GMGLShaderProgram();

	void load();

	void useProgram();
	GLuint getProgram() { D(d); return d->shaderProgram; }

	void attachShader(const GMGLShaderInfo& shaderCfgs);

private:
	void setProgram(GLuint program) { D(d); d->shaderProgram = program; }
	void removeShaders();
};

END_NS
#endif