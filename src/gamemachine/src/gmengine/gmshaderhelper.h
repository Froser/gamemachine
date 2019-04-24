#ifndef __GMSHADERHELPER_H__
#define __GMSHADERHELPER_H__
#include <gmcommon.h>
BEGIN_NS

struct GM_EXPORT GMShaderHelper
{
	static void loadShader(const IRenderContext* context);

	static void loadExtensionShaders(const IRenderContext* context);
};

END_NS
#endif