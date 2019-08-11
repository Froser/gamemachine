#ifndef __GMGLHELPER_H__
#define __GMGLHELPER_H__
#include <gmcommon.h>
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

	static bool loadShader(
		const IRenderContext* context,
		const GMGLShaderContent& forwardVertex,
		const GMGLShaderContent& forwardPixel,
		const GMGLShaderContent& deferredGeometryVertex,
		const GMGLShaderContent& deferredGeometryPixel,
		const GMGLShaderContent& deferredLightVertex,
		const GMGLShaderContent& deferredLightPixel,
		const GMGLShaderContent& filtersVertex,
		const GMGLShaderContent& filtersPixel
	);

	static bool loadShader(
		const IRenderContext* context,
		const GMString& forwardVertexFilePath,
		const GMString& forwardPixelFilePath,
		const GMString& deferredGeometryVertexFilePath,
		const GMString& deferredGeometryPixelFilePath,
		const GMString& deferredLightVertexFilePath,
		const GMString& deferredLightPixelFilePath,
		const GMString& filtersVertexFilePath,
		const GMString& filtersPixelFilePath
	);

	static const GMGLShaderInfo& getDefaultShaderCode(GMShaderType);

	static void initOpenGL();

	static const GMGLInfo& getOpenGLInfo();

	static bool isOpenGLShaderLanguageES();

	static bool isSupportGeometryShader();
};

END_NS
#endif
