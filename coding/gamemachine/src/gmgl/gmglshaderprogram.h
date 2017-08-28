#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "common.h"
#include "foundation/vector.h"
BEGIN_NS

struct GMGLShaderInfo
{
	GLenum type;
	GMString source;
	GMString filename;
};

typedef AlignedVector<GLuint> GMGLShaderIDList;
typedef AlignedVector<GMGLShaderInfo> GMGLShaderInfos;

GM_PRIVATE_OBJECT(GMGLShaderProgram)
{
	GMGLShaderInfos shaderInfos;
	GMGLShaderIDList shaders;
	GLuint shaderProgram = 0;
	static GLuint lastUsedProgram;
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
	void setMatrix4(const char* name, const GMfloat value[16]);
	void setVec4(const char* name, const GMfloat value[4]);
	void setVec3(const char* name, const GMfloat value[3]);
	void setInt(const char* name, GMint value);
	void setFloat(const char* name, GMfloat value);
	void setBool(const char* name, bool value);

	void setMatrix4(const GMString& name, const GMfloat value[16]);
	void setVec4(const GMString& name, const GMfloat value[4]);
	void setVec3(const GMString& name, const GMfloat value[3]);
	void setInt(const GMString& name, GMint value);
	void setFloat(const GMString& name, GMfloat value);
	void setBool(const GMString& name, bool value);

private:
	void setProgram(GLuint program) { D(d); d->shaderProgram = program; }
	void removeShaders();
	void expandSource(REF GMGLShaderInfo& shaderInfo);
	GMString expandSource(const GMString& workingDir, const GMString& source);
	bool matchMarco(const GMString& source, const GMString& marco, REF GMString& result);
	void expandInclude(const GMString& workingDir, const GMString& fn, IN OUT GMString& source);
};

END_NS
#endif