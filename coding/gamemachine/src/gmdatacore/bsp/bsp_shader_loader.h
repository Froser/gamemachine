#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include "common.h"
#include <string>
#include <map>
#include "bsp_render.h"

class TiXmlElement;
class TiXmlDocument;

BEGIN_NS
// 解析一些纹理配置，如天空、特效、动画等，并加入世界
class BSPGameWorld;
struct Shader;
struct ITexture;
class BSPShaderLoader
{
public:
	BSPShaderLoader();
	~BSPShaderLoader();

public:
	void init(const char* directory, BSPGameWorld* world, BSPRenderData* bspRenderData);
	void load();
	bool findItem(const char* name, GMuint lightmapId, REF Shader* shader);

	// parsers:
private:
	ITexture* addTextureToTextureContainer(const char* name);
	void parse(const char* filename);
	void parseItem(TiXmlElement* elem, GMuint lightmapId, REF Shader* shaderPtr);
	void parseStart();
	void parseEnd();

private:
	void parse_surfaceparm(Shader& shader, TiXmlElement* elem);
	void parse_cull(Shader& shader, TiXmlElement* elem);
	void parse_blendFunc(Shader& shader, TiXmlElement* elem);
	void parse_animMap(Shader& shader, TiXmlElement* elem);
	void parse_src(Shader& shader, TiXmlElement* elem, GMuint i);
	void parse_clampmap(Shader& shader, TiXmlElement* elem);
	void parse_map(Shader& shader, TiXmlElement* elem);
	void parse_map_tcMod(Shader& shader, TiXmlElement* elem);
	void parse_map_fromLightmap(Shader& shader, TiXmlElement* elem);
	void parse_normalmap(Shader& shader, TiXmlElement* elem);
	void parse_lights(Shader& shader, TiXmlElement* elem);
	void parse_light(Shader& shader, TiXmlElement* elem);

private:
	void createSky(Shader& shader);

private:
	std::string m_directory;
	BSPGameWorld* m_world;
	BSPRenderData* m_bspRender;
	std::map<std::string, TiXmlElement*> m_items;
	std::vector<TiXmlDocument*> m_shaderDocs;

	// 纹理编号，从TEXTURE_INDEX_AMBIENT开始
	GMint m_textureNum;
	GMint m_lightmapId;
};

END_NS
#endif