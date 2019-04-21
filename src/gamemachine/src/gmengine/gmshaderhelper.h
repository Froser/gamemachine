#ifndef __GMSHADERHELPER_H__
#define __GMSHADERHELPER_H__
#include <gmcommon.h>
BEGIN_NS

struct GMShaderHelper
{
	struct ShaderHelperResult
	{
		IComputeShaderProgram* cullShaderProgram = nullptr;
	};

	static void loadShader(const IRenderContext* context, ShaderHelperResult* result);
};

END_NS
#endif