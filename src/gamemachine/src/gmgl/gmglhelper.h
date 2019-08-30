#ifndef __GMGLHELPER_H__
#define __GMGLHELPER_H__
#include <gmcommon.h>
#include <gmxml.h>
BEGIN_NS

struct GMGLShaderInfo;
constexpr GMint32 MAX_NUM_SHADING_LANGUAGE_VERSIONS = 32;

struct GMGLInfo
{
	GMString vendor;
	GMString renderer;
	GMString version;
	GMString shadingLanguageVersions;
	GMString extensions;
};

struct GM_EXPORT GMGLHelper
{
	struct GMGLShaderContent
	{
		GMString code;
		GMString path;
	};

	static bool loadShader(const IRenderContext* context, const GMString& manifest);

	static Vector<GMGLShaderInfo> getDefaultShaderCodes(GMShaderType);

	static Vector<GMGLShaderInfo> getDefaultShaderIncludes(GMShaderType);

	static void initOpenGL();

	static bool isOpenGLShaderLanguageES();

	static bool isSupportGeometryShader();
};

END_NS
#endif
