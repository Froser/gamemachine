#ifndef __MTLREADER_PRIVATE_H__
#include "common.h"
#include <string>
#include <map>
BEGIN_NS
typedef std::string MaterialName;

struct MaterialProperties
{
	Ffloat Ns;
	Ffloat d;
	Ffloat Tr;
	Ffloat Tf_r, Tf_g, Tf_b;
	Fint illum;
	Ffloat Ka_r, Ka_g, Ka_b;
	Ffloat Kd_r, Kd_g, Kd_b;
	Ffloat Ks_r, Ks_g, Ks_b;
};

typedef std::map<MaterialName, MaterialProperties> Materials;

class MtlReaderPrivate
{
	friend class MtlReader;

private:
	MtlReaderPrivate() : m_currentMaterial(nullptr) {}
	void parseLine(const char* line);
	Materials& getMaterials();

private:
	MaterialProperties* m_currentMaterial;
	Materials m_materials;
};
END_NS
#endif