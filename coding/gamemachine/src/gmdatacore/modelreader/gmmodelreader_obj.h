#ifndef __MODELREADER_OBJ_H__
#define __MODELREADER_OBJ_H__
#include "common.h"
#include "gmmodelreader.h"
#include "foundation/linearmath.h"
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

class Object;
struct GMBuffer;
class Scanner;
class Component;
GM_PRIVATE_OBJECT(GMModelReader_Obj)
{
	Object* object;
	AlignedVector<linear_math::Vector3> vertices;
	AlignedVector<linear_math::Vector3> normals;
	AlignedVector<linear_math::Vector2> textures;
	Map<GMString, ModelReader_Obj_Material> materials;
	GMString currentMaterialName;
	Component* currentComponent;
};

// 一个Obj文件只由一个部分组成，不存在骨骼等动画，是刚体静态的
class GMModelReader_Obj : public GMObject, public IModelReader
{
	DECLARE_PRIVATE(GMModelReader_Obj);

public:
	GMModelReader_Obj();
	~GMModelReader_Obj();

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT Object** object) override;
	virtual bool test(const GMBuffer& buffer) override;

private:
	void init();
	void appendFace(Scanner& scanner);
	void loadMaterial(const GMModelLoadSettings& settings, const char* mtlFilename);
	void applyMaterial(const ModelReader_Obj_Material& material, Shader& shader);
	ModelReader_Obj_Material* getMaterial(const GMString& materialName);
};

END_NS
#endif