#ifndef __MODELREADER_OBJ_H__
#define __MODELREADER_OBJ_H__
#include <gmcommon.h>
#include "gmmodelreader.h"
#include <linearmath.h>
#include <map>
#include "foundation/vector.h"

BEGIN_NS

struct ModelReader_Obj_Material
{
	GMfloat ns;
	GMfloat kd[3];
	GMfloat ka[3];
	GMfloat ks[3];
};

class GMModel;
struct GMBuffer;
class Scanner;
class GMComponent;
GM_PRIVATE_OBJECT(GMModelReader_Obj)
{
	GMModel* model = nullptr;
	AlignedVector<glm::vec3> positions;
	AlignedVector<glm::vec3> normals;
	AlignedVector<glm::vec2> textures;
	Map<GMString, ModelReader_Obj_Material> materials;
	GMString currentMaterialName;
	GMComponent* currentComponent;
};

// 一个Obj文件只由一个部分组成，不存在骨骼等动画，是刚体静态的
class GMModelReader_Obj : public GMObject, public IModelReader
{
	DECLARE_PRIVATE(GMModelReader_Obj);

public:
	GMModelReader_Obj();
	~GMModelReader_Obj();

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModel** object) override;
	virtual bool test(const GMBuffer& buffer) override;

private:
	void init();
	void appendFace(Scanner& scanner);
	void loadMaterial(const GMModelLoadSettings& settings, const char* mtlFilename);
	void applyMaterial(const ModelReader_Obj_Material& material, GMShader& shader);
	ModelReader_Obj_Material* getMaterial(const GMString& materialName);
};

END_NS
#endif