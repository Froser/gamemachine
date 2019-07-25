#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include <gmcommon.h>
#include "gmbsp_render.h"
#include <gmxml.h>

BEGIN_NS
// 解析一些纹理配置，如天空、特效、动画等，并加入世界
class GMBSPGameWorld;

GM_PRIVATE_CLASS(GMBSPShaderLoader);
class GMBSPShaderLoader
{
	GM_DECLARE_PRIVATE(GMBSPShaderLoader)
	GM_DISABLE_ASSIGN(GMBSPShaderLoader)

public:
	GMBSPShaderLoader();
	~GMBSPShaderLoader();

public:
	void init(GMBSPGameWorld* world, GMBSPRenderData* bspRenderData);
	void load();
	bool findItem(const GMString& name, GMint32 lightmapId, REF GMShader* shader);

	// parsers:
private:
	GMTextureAsset addTextureToTextureContainer(const GMString& name);
	void parse(const char* buffer);
	void parseItem(GMXMLElement* elem, GMint32 lightmapId, REF GMShader* shaderPtr);
	void parseStart();
	void parseEnd();

private:
	void parse_surfaceparm(GMShader& shader, GMXMLElement* elem);
	void parse_cull(GMShader& shader, GMXMLElement* elem);
	void parse_blendFunc(GMShader& shader, GMXMLElement* elem);
	void parse_animMap(GMShader& shader, GMXMLElement* elem);
	void parse_src(GMShader& shader, GMXMLElement* elem);
	void parse_clampmap(GMShader& shader, GMXMLElement* elem);
	void parse_map(GMShader& shader, GMXMLElement* elem);
	void parse_map_tcMod(GMShader& shader, GMXMLElement* elem);
	void parse_map_fromLightmap(GMShader& shader, GMXMLElement* elem);
	void parse_normalmap(GMShader& shader, GMXMLElement* elem);
	void parse_lights(GMShader& shader, GMXMLElement* elem);
	void parse_light(GMShader& shader, GMXMLElement* elem);

private:
	void createSky(GMShader& shader);
};

END_NS
#endif