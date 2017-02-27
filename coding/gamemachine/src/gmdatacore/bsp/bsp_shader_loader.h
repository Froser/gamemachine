#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include "common.h"
#include <string>

class TiXmlElement;

BEGIN_NS
// 解析一些纹理配置，如天空、特效、动画等，并加入世界
class BSPGameWorld;
struct Shader;
class BSPShaderLoader
{
public:
	BSPShaderLoader(const char* directory, BSPGameWorld& world);

public:
	void parseAll();

private:
	void parse(const char* filename);
	void parseItem(TiXmlElement* elem);
	void parse_surfaceparm(Shader& shader, TiXmlElement* elem);
	void parse_cull(Shader& shader, TiXmlElement* elem);
	void parse_animMap(Shader& shader, TiXmlElement* elem);
	void parse_src(Shader& shader, TiXmlElement* elem, GMuint i);

private:
	std::string m_directory;
	BSPGameWorld& m_world;
};

END_NS
#endif