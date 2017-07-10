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

public:
	void setMatrix4(const GMString& name, const GMfloat value[16]);
	void setVec4(const GMString& name, const GMfloat value[4]);
	void setVec3(const GMString& name, const GMfloat value[3]);
	void setInt(const GMString& name, GMint value);
	void setFloat(const GMString& name, GMfloat value);
	void setBool(const GMString& name, bool value);

private:
	void setProgram(GLuint program) { D(d); d->shaderProgram = program; }
	void removeShaders();
};

END_NS
#endif