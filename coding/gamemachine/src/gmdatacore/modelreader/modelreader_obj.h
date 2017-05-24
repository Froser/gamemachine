#ifndef __MODELREADER_OBJ_H__
#define __MODELREADER_OBJ_H__
#include "common.h"
#include "modelreader.h"
#include "utilities/linearmath.h"
#include <map>
#include "utilities/vector.h"

BEGIN_NS

struct ModelReader_Obj_Material
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

	GMfloat ns;
	linear_math::Vector3 kd;
	linear_math::Vector3 ka;
	linear_math::Vector3 ks;
};

class Object;
struct GamePackageBuffer;
class Scanner;
class Component;
struct Shader;
struct ModelReader_ObjPrivate
{
	~ModelReader_ObjPrivate()
	{
		for (auto iter = materials.begin(); iter != materials.end(); iter++)
		{
			if ((*iter).second)
				delete (*iter).second;
		}
	}

	Object* object;
	AlignedVector<linear_math::Vector3> vertices;
	AlignedVector<linear_math::Vector3> normals;
	AlignedVector<linear_math::Vector2> textures;
	std::map<std::string, ModelReader_Obj_Material*> materials;
	std::string currentMaterialName;
	Component* currentComponent;
};

// 一个Obj文件只由一个部分组成，不存在骨骼等动画，是刚体静态的
class ModelReader_Obj : public IModelReader
{
	DEFINE_PRIVATE(ModelReader_Obj);

public:
	ModelReader_Obj();

public:
	virtual bool load(const ModelLoadSettings& settings, GamePackageBuffer& buffer, OUT Object** object) override;
	virtual bool test(const GamePackageBuffer& buffer) override;

private:
	void init();
	void appendFace(Scanner& scanner);
	void loadMaterial(const ModelLoadSettings& settings, const char* mtlFilename);
	void applyMaterial(const ModelReader_Obj_Material& material, Shader& shader);
	ModelReader_Obj_Material* getMaterial(const std::string& materialName);
};

END_NS
#endif