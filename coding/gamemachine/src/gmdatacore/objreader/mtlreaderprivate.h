#ifndef __MTLREADER_PRIVATE_H__
#define __MTLREADER_PRIVATE_H__
#include "common.h"
#include <string>
#include <map>
#include "gmdatacore/object.h"

BEGIN_NS
typedef std::string MaterialName;
struct MaterialProperties
{
	GMfloat Ns;
	GMfloat d;
	GMfloat Tr;
	GMfloat Tf_r, Tf_g, Tf_b;
	GMint illum;
	GMfloat Ka_r, Ka_g, Ka_b;
	bool Ka_switch : 1;
	GMfloat Kd_r, Kd_g, Kd_b;
	bool Kd_switch : 1;
	GMfloat Ks_r, Ks_g, Ks_b;
	bool Ks_switch : 1;
	bool map_Ka_switch : 1;
	char map_Ka[LINE_MAX];
	bool map_Kd_switch : 1;
	char map_Kd[LINE_MAX];
	bool map_Bump_switch : 1;
	char map_Bump[LINE_MAX];
};

typedef std::map<MaterialName, MaterialProperties> Materials;

class MtlReaderPrivate
{
	friend class MtlReader;

private:
	MtlReaderPrivate() : m_pCurrentMaterial(nullptr) {}
	void parseLine(const char* line);
	Materials& getMaterials();
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }

private:
	MaterialProperties* m_pCurrentMaterial;
	Materials m_materials;
	std::string m_workingDir;
};
END_NS
#endif