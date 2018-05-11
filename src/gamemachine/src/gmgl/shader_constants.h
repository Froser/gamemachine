#ifndef __GMGL_SHADER_CONSTAINS_H__
#define __GMGL_SHADER_CONSTAINS_H__
#include <gmcommon.h>
#include <gmshader.h>
#include "foundation/assert.h"
#include <gmgraphicengine.h>

BEGIN_NS

#define		GMSHADER(memberName)				"GM_" memberName

//位置
#define		GMSHADER_SHADOW_MATRIX				GMSHADER("shadow_matrix")

//绘制调试
#define		GMSHADER_DEBUG_DRAW_NORMAL			GMSHADER("debug_draw_normal")

//特效
#define		GMSHADER_FRAMEBUFFER				GMSHADER("framebuffer")

// 获取名称辅助函数
// 字符串拼接太慢，因此采用一些预设的字符串

constexpr GMint GMGL_MAX_UNIFORM_NAME_LEN = 64; //uniform最长名称
constexpr GMint GMGL_MAX_LIGHT_COUNT = 10; //灯光最大数量

inline const char* getTextureUniformName(const GMShaderVariablesDesc* desc, GMTextureType t, GMuint index)
{
	GM_ASSERT(index < GMMaxTextureCount(t));
	static const std::string GMSHADER_AMBIENT_TEXTURES_0 = std::string(desc->AmbientTextureName) + "[0]";
	static const std::string GMSHADER_AMBIENT_TEXTURES_1 = std::string(desc->AmbientTextureName) + "[1]";
	static const std::string GMSHADER_AMBIENT_TEXTURES_2 = std::string(desc->AmbientTextureName) + "[2]";
	static const std::string GMSHADER_DIFFUSE_TEXTURES_0 = std::string(desc->DiffuseTextureName) + "[0]";
	static const std::string GMSHADER_DIFFUSE_TEXTURES_1 = std::string(desc->DiffuseTextureName) + "[1]";
	static const std::string GMSHADER_DIFFUSE_TEXTURES_2 = std::string(desc->DiffuseTextureName) + "[2]";
	static const std::string GMSHADER_SPECULAR_TEXTURES_0 = std::string(desc->SpecularTextureName) + "[0]";
	static const std::string GMSHADER_SPECULAR_TEXTURES_1 = std::string(desc->SpecularTextureName) + "[1]";
	static const std::string GMSHADER_SPECULAR_TEXTURES_2 = std::string(desc->SpecularTextureName) + "[2]";
	static const std::string GMSHADER_NORMALMAP_TEXTURE_0 = std::string(desc->NormalMapTextureName) + "[0]";
	static const std::string GMSHADER_LIGHTMAP_TEXTURE_0 = std::string(desc->LightMapTextureName) + "[0]";

	switch (t)
	{
	case GMTextureType::Ambient:
		return index == 0 ? GMSHADER_AMBIENT_TEXTURES_0.c_str() :
			index == 1 ? GMSHADER_AMBIENT_TEXTURES_1.c_str() :
			index == 2 ? GMSHADER_AMBIENT_TEXTURES_2.c_str() : "";
	case GMTextureType::Diffuse:
		return index == 0 ? GMSHADER_DIFFUSE_TEXTURES_0.c_str() :
			index == 1 ? GMSHADER_DIFFUSE_TEXTURES_1.c_str() :
			index == 2 ? GMSHADER_DIFFUSE_TEXTURES_2.c_str() : "";
	case GMTextureType::Specular:
		return index == 0 ? GMSHADER_SPECULAR_TEXTURES_0.c_str() :
			index == 1 ? GMSHADER_SPECULAR_TEXTURES_1.c_str() :
			index == 2 ? GMSHADER_SPECULAR_TEXTURES_2.c_str() : "";
	case GMTextureType::NormalMap:
		return index == 0 ? GMSHADER_NORMALMAP_TEXTURE_0.c_str() : "";
	case GMTextureType::Lightmap:
		return index == 0 ? GMSHADER_LIGHTMAP_TEXTURE_0.c_str() : "";
	case GMTextureType::EndOfCommonTexture:
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

template <typename CharType, size_t ArraySize>
void combineUniform(REF CharType (&dest)[ArraySize], const char* srcA, const char* srcB)
{
	GMString::stringCopy(dest, srcA);
	GMString::stringCat(dest, srcB);
}

template <typename CharType, size_t ArraySize>
void combineUniform(REF CharType(&dest)[ArraySize], const char* srcA, const char* srcB, const char* srcC)
{
	GMString::stringCopy(dest, srcA);
	GMString::stringCat(dest, srcB);
	GMString::stringCat(dest, srcC);
}

END_NS
#endif
