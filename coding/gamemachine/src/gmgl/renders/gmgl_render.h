#ifndef __GMGL_RENDER_H__
#define __GMGL_RENDER_H__
#include "common.h"
BEGIN_NS

struct IGraphicEngine;
class ChildObject;
struct Shader;
struct IRender
{
	virtual void begin(IGraphicEngine* engine, ChildObject* childObj) = 0;
	virtual void beginShader(Shader& shader) = 0;
	virtual void endShader() = 0;
	virtual void end() = 0;
};

END_NS
#endif