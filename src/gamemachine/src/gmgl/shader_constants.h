#ifndef __SHADER_CONSTAINS_H__
#define __SHADER_CONSTAINS_H__
#include <gmcommon.h>
#include <gmshader.h>
#include "foundation/assert.h"

BEGIN_NS

#define		GMSHADER(memberName)				"GM_" memberName

//状态
#define		GMSHADER_SHADER_TYPE				GMSHADER("shader_type")
#define		GMSHADER_SHADER_PROC				GMSHADER("shader_proc")

//位置
#define		GMSHADER_MODEL_MATRIX				GMSHADER("model_matrix")
#define		GMSHADER_SHADOW_MATRIX				GMSHADER("shadow_matrix")
#define		GMSHADER_PROJECTION_MATRIX			GMSHADER("projection_matrix")
#define		GMSHADER_INV_TRANS_MODEL_MATRIX		GMSHADER("inverse_transpose_model_matrix")
#define		GMSHADER_VIEW_MATRIX				GMSHADER("view_matrix")
#define		GMSHADER_INVERSE_VIEW_MATRIX		GMSHADER("inverse_view_matrix")
#define		GMSHADER_VIEW_POSITION				GMSHADER("view_position")

//纹理
#define		GMSHADER_CUBEMAP_TEXTURE			GMSHADER("cubemap")
#define		GMSHADER_AMBIENT_TEXTURES(i)		GMSHADER("ambient_textures") "[" #i "]"
#define		GMSHADER_DIFFUSE_TEXTURES(i)		GMSHADER("diffuse_textures") "[" #i "]"
#define		GMSHADER_NORMALMAP_TEXTURES(i)		GMSHADER("normalmap_textures") "[" #i "]"
#define		GMSHADER_LIGHTMAP_TEXTURES(i)		GMSHADER("lightmap_textures") "[" #i "]"
#define		GMSHADER_TEXTURES_TEXTURE			".texture"
#define		GMSHADER_TEXTURES_ENABLED			".enabled"
#define		GMSHADER_TEXTURES_SCROLL_S			".scroll_s"
#define		GMSHADER_TEXTURES_SCROLL_T			".scroll_t"
#define		GMSHADER_TEXTURES_SCALE_S			".scale_s"
#define		GMSHADER_TEXTURES_SCALE_T			".scale_t"

//光照
#define		GMSHADER_AMBIENT_LIGHTS(i)			GMSHADER("ambients") "[" #i "]"
#define		GMSHADER_AMBIENTS_COUNT				GMSHADER("ambients_count")
#define		GMSHADER_SPECULAR_LIGHTS(i)			GMSHADER("speculars") "[" #i "]"
#define		GMSHADER_SPECULARS_COUNT			GMSHADER("speculars_count")
#define		GMSHADER_LIGHTS_LIGHTCOLOR			".lightColor"
#define		GMSHADER_LIGHTS_LIGHTPOSITION		".lightPosition"

//材质
#define		GMSHADER_MATERIAL_KA				GMSHADER("material.ka")
#define		GMSHADER_MATERIAL_KD				GMSHADER("material.kd")
#define		GMSHADER_MATERIAL_KS				GMSHADER("material.ks")
#define		GMSHADER_MATERIAL_SHININESS			GMSHADER("material.shininess")
#define		GMSHADER_MATERIAL_REFRACTIVITY		GMSHADER("material.refractivity")

//绘制调试
#define		GMSHADER_DEBUG_DRAW_NORMAL			GMSHADER("debug_draw_normal")

//特效
#define		GMSHADER_FRAMEBUFFER				GMSHADER("framebuffer")
#define		GMSHADER_EFFECTS_TEXTURE_OFFSET_X	GMSHADER("effects_texture_offset_x")
#define		GMSHADER_EFFECTS_TEXTURE_OFFSET_Y	GMSHADER("effects_texture_offset_y")
#define		GMSHADER_EFFECTS_NONE				GMSHADER("effects_none")
#define		GMSHADER_EFFECTS_INVERSION			GMSHADER("effects_inversion")
#define		GMSHADER_EFFECTS_SHARPEN			GMSHADER("effects_sharpen")
#define		GMSHADER_EFFECTS_BLUR				GMSHADER("effects_blur")
#define		GMSHADER_EFFECTS_GRAYSCALE			GMSHADER("effects_grayscale")
#define		GMSHADER_EFFECTS_EDGEDETECT			GMSHADER("effects_edgedetect")

// 获取名称辅助函数
// 字符串拼接太慢，因此采用一些预设的字符串

constexpr GMint GMGL_MAX_UNIFORM_NAME_LEN = 64; //uniform最长名称
constexpr GMint GMGL_MAX_LIGHT_COUNT = 10; //灯光最大数量

inline const char* getTextureUniformName(GMTextureType t, GMint index)
{
	GM_ASSERT(index < GMMaxTextureCount(t));
	switch (t)
	{
	case GMTextureType::AMBIENT:
		return index == 0 ? GMSHADER_AMBIENT_TEXTURES(0) :
			index == 1 ? GMSHADER_AMBIENT_TEXTURES(1) :
			index == 2 ? GMSHADER_AMBIENT_TEXTURES(2) : "";
	case GMTextureType::DIFFUSE:
		return index == 0 ? GMSHADER_DIFFUSE_TEXTURES(0) :
			index == 1 ? GMSHADER_DIFFUSE_TEXTURES(1) :
			index == 2 ? GMSHADER_DIFFUSE_TEXTURES(2) : "";
	case GMTextureType::NORMALMAP:
		return index == 0 ? GMSHADER_NORMALMAP_TEXTURES(0) :
			index == 1 ? GMSHADER_NORMALMAP_TEXTURES(1) :
			index == 2 ? GMSHADER_NORMALMAP_TEXTURES(2) : "";
	case GMTextureType::LIGHTMAP:
		return index == 0 ? GMSHADER_LIGHTMAP_TEXTURES(0) :
			index == 1 ? GMSHADER_LIGHTMAP_TEXTURES(1) :
			index == 2 ? GMSHADER_LIGHTMAP_TEXTURES(2) : "";
	case GMTextureType::END:
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

inline const char* getLightUniformName(GMLightType t, GMint index)
{
	GM_ASSERT(index < GMGL_MAX_LIGHT_COUNT);
	switch (t)
	{
	case GMLightType::AMBIENT:
		return index == 0 ? GMSHADER_AMBIENT_LIGHTS(0) :
			index == 1 ? GMSHADER_AMBIENT_LIGHTS(1) :
			index == 2 ? GMSHADER_AMBIENT_LIGHTS(2) :
			index == 3 ? GMSHADER_AMBIENT_LIGHTS(3) :
			index == 4 ? GMSHADER_AMBIENT_LIGHTS(4) :
			index == 5 ? GMSHADER_AMBIENT_LIGHTS(5) :
			index == 6 ? GMSHADER_AMBIENT_LIGHTS(6) :
			index == 7 ? GMSHADER_AMBIENT_LIGHTS(7) :
			index == 8 ? GMSHADER_AMBIENT_LIGHTS(8) :
			index == 9 ? GMSHADER_AMBIENT_LIGHTS(9) :
			index == 10 ? GMSHADER_AMBIENT_LIGHTS(10) : "";
	case GMLightType::SPECULAR:
		return index == 0 ? GMSHADER_SPECULAR_LIGHTS(0) :
			index == 1 ? GMSHADER_SPECULAR_LIGHTS(1) :
			index == 2 ? GMSHADER_SPECULAR_LIGHTS(2) :
			index == 3 ? GMSHADER_SPECULAR_LIGHTS(3) :
			index == 4 ? GMSHADER_SPECULAR_LIGHTS(4) :
			index == 5 ? GMSHADER_SPECULAR_LIGHTS(5) :
			index == 6 ? GMSHADER_SPECULAR_LIGHTS(6) :
			index == 7 ? GMSHADER_SPECULAR_LIGHTS(7) :
			index == 8 ? GMSHADER_SPECULAR_LIGHTS(8) :
			index == 9 ? GMSHADER_SPECULAR_LIGHTS(9) :
			index == 10 ? GMSHADER_SPECULAR_LIGHTS(10) : "";
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

inline void combineUniform(REF char* dest, const char* srcA, const char* srcB)
{
	strcpy_s(dest, GMGL_MAX_UNIFORM_NAME_LEN, srcA);
	strcat_s(dest, GMGL_MAX_UNIFORM_NAME_LEN, srcB);
}

END_NS
#endif
