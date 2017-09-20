#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include <gmcommon.h>
#include <gmmodel.h>
BEGIN_NS

class GMModel;
class Shader;

GM_INTERFACE(IPrimitiveCreatorShaderCallback)
{
	virtual void onCreateShader(Shader& shader) = 0;
};

struct GMPrimitiveCreator
{
	static void createCube(GMfloat extents[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMMeshType type = GMMeshType::Model3D);
	static void createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMMeshType type = GMMeshType::Model3D);
};

END_NS
#endif