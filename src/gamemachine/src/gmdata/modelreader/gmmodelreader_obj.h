#ifndef __MODELREADER_OBJ_H__
#define __MODELREADER_OBJ_H__
#include <gmcommon.h>
#include "gmmodelreader.h"
#include <linearmath.h>
#include <map>
#include <gmgameobject.h>

BEGIN_NS

struct ModelReader_Obj_Material
{
	GMfloat ns;
	GMfloat kd[3];
	GMfloat ka[3];
	GMfloat ks[3];
	GMString map_Ka;
	GMString map_Kd;
};

class GMModel;
struct GMBuffer;
class GMScanner;
class GMMesh;
GM_PRIVATE_OBJECT(GMModelReader_Obj)
{
	GMModels* models = nullptr;
	AlignedVector<GMVec3> positions;
	AlignedVector<GMVec3> normals;
	AlignedVector<GMVec2> texcoords;
	Map<GMString, ModelReader_Obj_Material> materials;
	HashMap<GMString, GMAsset, GMStringHashFunctor> shaders;
	GMString currentMaterialName;
	GMModel* currentModel;
};

// 一个Obj文件只由一个部分组成，不存在骨骼等动画，是刚体静态的
class GMModelReader_Obj : public GMObject, public IModelReader
{
	GM_DECLARE_PRIVATE(GMModelReader_Obj);

public:
	GMModelReader_Obj();
	~GMModelReader_Obj();

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
	virtual bool test(const GMBuffer& buffer) override;

private:
	void init();
	void appendFace(const GMModelLoadSettings& settings, GMScanner& scanner);
	void loadMaterial(const GMModelLoadSettings& settings, const GMString& mtlFilename);
	void applyMaterial(const GMModelLoadSettings& settings, const ModelReader_Obj_Material& material, GMShader& shader);
	ModelReader_Obj_Material* getMaterial(const GMString& materialName);
};

END_NS
#endif