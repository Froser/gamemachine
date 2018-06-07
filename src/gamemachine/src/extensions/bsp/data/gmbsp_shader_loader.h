#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include <gmcommon.h>
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
	GMint lightmapId;
};


GM_ALIGNED_16(class) GMBSPShaderLoader : public GMObject
{
	GM_DECLARE_PRIVATE(GMBSPShaderLoader)

public:
	GMBSPShaderLoader();
	~GMBSPShaderLoader();

public:
	void init(const GMString& directory, GMBSPGameWorld* world, GMBSPRenderData* bspRenderData);
	void load();
	bool findItem(const GMString& name, GMint lightmapId, REF GMShader* shader);

	// parsers:
private:
	ITexture* addTextureToTextureContainer(const GMString& name);
	void parse(const char* buffer);
	void parseItem(TiXmlElement* elem, GMint lightmapId, REF GMShader* shaderPtr);
	void parseStart();
	void parseEnd();

private:
	void parse_surfaceparm(GMShader& shader, TiXmlElement* elem);
	void parse_cull(GMShader& shader, TiXmlElement* elem);
	void parse_blendFunc(GMShader& shader, TiXmlElement* elem);
	void parse_animMap(GMShader& shader, TiXmlElement* elem);
	void parse_src(GMShader& shader, TiXmlElement* elem);
	void parse_clampmap(GMShader& shader, TiXmlElement* elem);
	void parse_map(GMShader& shader, TiXmlElement* elem);
	void parse_map_tcMod(GMShader& shader, TiXmlElement* elem);
	void parse_map_fromLightmap(GMShader& shader, TiXmlElement* elem);
	void parse_normalmap(GMShader& shader, TiXmlElement* elem);
	void parse_lights(GMShader& shader, TiXmlElement* elem);
	void parse_light(GMShader& shader, TiXmlElement* elem);

private:
	void createSky(GMShader& shader);
};

END_NS
#endif