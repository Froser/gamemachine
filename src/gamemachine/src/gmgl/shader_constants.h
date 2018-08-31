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

inline const char* getTextureUniformName(GMTextureType t)
{
	static const std::string GMSHADER_AMBIENT_TEXTURES = std::string(GM_VariablesDesc.AmbientTextureName);
	static const std::string GMSHADER_DIFFUSE_TEXTURES = std::string(GM_VariablesDesc.DiffuseTextureName);
	static const std::string GMSHADER_SPECULAR_TEXTURES = std::string(GM_VariablesDesc.SpecularTextureName);
	static const std::string GMSHADER_NORMALMAP_TEXTURE = std::string(GM_VariablesDesc.NormalMapTextureName);
	static const std::string GMSHADER_LIGHTMAP_TEXTURE = std::string(GM_VariablesDesc.LightMapTextureName);
	static const std::string GMSHADER_ALBEDO_TEXTURE = std::string(GM_VariablesDesc.AlbedoTextureName);
	static const std::string GMSHADER_METALLIC_ROUGHNESS_AO_TEXTURE = std::string(GM_VariablesDesc.MetallicRoughnessAOTextureName);

	switch (t)
	{
	case GMTextureType::Ambient:
		return GMSHADER_AMBIENT_TEXTURES.c_str();
	case GMTextureType::Diffuse:
		return GMSHADER_DIFFUSE_TEXTURES.c_str();
	case GMTextureType::Specular:
		return GMSHADER_SPECULAR_TEXTURES.c_str();
	case GMTextureType::NormalMap:
		return GMSHADER_NORMALMAP_TEXTURE.c_str();
	case GMTextureType::Lightmap:
		return GMSHADER_LIGHTMAP_TEXTURE.c_str();
	case GMTextureType::Albedo:
		return GMSHADER_ALBEDO_TEXTURE.c_str();
	case GMTextureType::MetallicRoughnessAO:
		return GMSHADER_METALLIC_ROUGHNESS_AO_TEXTURE.c_str();
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
