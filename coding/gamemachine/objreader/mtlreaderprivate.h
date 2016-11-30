#ifndef __MTLREADER_PRIVATE_H__
#define __MTLREADER_PRIVATE_H__
#include "common.h"
#include <string>
#include <map>

BEGIN_NS
typedef std::string MaterialName;

const int TEXTURE_ERROR = 0;
struct MaterialProperties
{
	GMuint textureID;
	bool hasTexture : 1;
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
};

typedef std::map<MaterialName, MaterialProperties> Materials;

class Image;
struct TextureInfo
{
	Image* texture;
	GMuint id;
};

struct IObjReaderCallback;
class MtlReaderPrivate
{
	friend class MtlReader;

private:
	MtlReaderPrivate() : m_pCurrentMaterial(nullptr) {}
	~MtlReaderPrivate();
	void parseLine(const char* line);
	Materials& getMaterials();
	void setWorkingDir(const std::string& workingDir) { m_workingDir = workingDir; }
	void setCallback(IObjReaderCallback* callback) { m_pCallback = callback; }

private:
	MaterialProperties* m_pCurrentMaterial;
	Materials m_materials;
	std::string m_workingDir;
	IObjReaderCallback* m_pCallback;
	std::map<std::string, TextureInfo> m_texMap;
};
END_NS
#endif