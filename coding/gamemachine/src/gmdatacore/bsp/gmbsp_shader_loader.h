#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include <gmcommon.h>
#include <string>
#include <map>
#include "gmbsp_render.h"

class TiXmlElement;
class TiXmlDocument;

BEGIN_NS
// 解析一些纹理配置，如天空、特效、动画等，并加入世界
class GMBSPGameWorld;

GM_PRIVATE_OBJECT(GMBSPShaderLoader)
{
	GMString directory;
	GMBSPGameWorld* world;
	GMBSPRenderData* bspRender;
	Map<GMString, TiXmlElement*> items;
	AlignedVector<TiXmlDocument*> shaderDocs;

	// 纹理编号，从TEXTURE_INDEX_AMBIENT开始
	GMint textureNum;
	GMint lightmapId;
};


GM_ALIGNED_16(class) GMBSPShaderLoader : public GMObject
{
	DECLARE_PRIVATE(GMBSPShaderLoader)

public:
	GMBSPShaderLoader();
	~GMBSPShaderLoader();

public:
	void init(const GMString& directory, GMBSPGameWorld* world, GMBSPRenderData* bspRenderData);
	void load();
	bool findItem(const GMString& name, GMint lightmapId, REF Shader* shader);

	// parsers:
private:
	ITexture* addTextureToTextureContainer(const char* name);
	void parse(const char* buffer);
	void parseItem(TiXmlElement* elem, GMint lightmapId, REF Shader* shaderPtr);
	void parseStart();
	void parseEnd();

private:
	void parse_surfaceparm(Shader& shader, TiXmlElement* elem);
	void parse_cull(Shader& shader, TiXmlElement* elem);
	void parse_blendFunc(Shader& shader, TiXmlElement* elem);
	void parse_animMap(Shader& shader, TiXmlElement* elem);
	void parse_src(Shader& shader, TiXmlElement* elem);
	void parse_clampmap(Shader& shader, TiXmlElement* elem);
	void parse_map(Shader& shader, TiXmlElement* elem);
	void parse_map_tcMod(Shader& shader, TiXmlElement* elem);
	void parse_map_fromLightmap(Shader& shader, TiXmlElement* elem);
	void parse_normalmap(Shader& shader, TiXmlElement* elem);
	void parse_lights(Shader& shader, TiXmlElement* elem);
	void parse_light(Shader& shader, TiXmlElement* elem);

private:
	void createSky(Shader& shader);
};

END_NS
#endif