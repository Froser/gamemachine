#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include <gmcommon.h>
#include <gmmodel.h>
BEGIN_NS

class GMModel;
class GMShader;

GM_INTERFACE(IPrimitiveCreatorShaderCallback)
{
	virtual void onCreateShader(GMShader& shader) = 0;
};

struct GMPrimitiveCreator
{
	enum GMCreateAnchor
	{
		TopLeft,
		Center,
	};

	static GMfloat* origin();

	static void createCube(GMfloat extents[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMMeshType type = GMMeshType::Model3D);
	static void createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMMeshType type = GMMeshType::Model3D, GMCreateAnchor anchor = Center, GMfloat (*customUV)[12] = nullptr);
};

struct GMPrimitiveUtil
{
	static void translateModelTo(REF GMModel& model, const GMfloat(&trans)[3]);
	static void scaleModel(REF GMModel& model, const GMfloat(&scaling)[3]);
};

END_NS
#endif