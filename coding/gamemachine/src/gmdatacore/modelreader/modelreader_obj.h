#ifndef __MODELREADER_OBJ_H__
#define __MODELREADER_OBJ_H__
#include "common.h"
#include "modelreader.h"
#include <vector>
#include "utilities/vmath.h"
#include <map>

BEGIN_NS

struct ModelReader_Obj_Material
{
	GMfloat ns;
	vmath::vec3 kd;
	vmath::vec3 ka;
	vmath::vec3 ks;
};

class Object;
struct GamePackageBuffer;
class Scanner;
class Component;
struct ModelReader_ObjPrivate
{
	Object* object;
	std::vector<vmath::vec3> vertices;
	std::vector<vmath::vec3> normals;
	std::vector<vmath::vec2> textures;
	std::map<std::string, ModelReader_Obj_Material> materials;
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
};

END_NS
#endif