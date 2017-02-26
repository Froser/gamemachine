#ifndef __BSP_SHADER_LOADER_H__
#define __BSP_SHADER_LOADER_H__
#include "common.h"
#include <string>
BEGIN_NS

// 解析一些纹理配置，如天空、特效、动画等
class BSPGameWorld;
class BSPShaderLoader
{
public:
	BSPShaderLoader(const char* directory, BSPGameWorld& world);

public:
	void parseAll();

private:
	std::string m_directory;
	BSPGameWorld& m_world;
};

END_NS
#endif