#ifndef __SHADER_CONSTAINS_H__
#define __SHADER_CONSTAINS_H__
#include "common.h"
BEGIN_NS

#define		GMSHADER(memberName)			"GM_" memberName

//位置
#define		GMSHADER_MODEL_MATRIX				GMSHADER("model_matrix")
#define		GMSHADER_SHADOW_MATRIX				GMSHADER("shadow_matrix")
#define		GMSHADER_PROJECTION_MATRIX			GMSHADER("projection_matrix")
#define		GMSHADER_VIEW_MATRIX				GMSHADER("view_matrix")
#define		GMSHADER_VIEW_POSITION				GMSHADER("view_position")

//纹理
#define		GMSHADER_AMBIENT_TEXTURES			GMSHADER("ambient_textures")
#define		GMSHADER_DIFFUSE_TEXTURES			GMSHADER("diffuse_textures")
#define		GMSHADER_NORMAL_MAPPING_TEXTURES	GMSHADER("normalmap_textures")
#define		GMSHADER_LIGHTMAP_TEXTURES			GMSHADER("lightmap_textures")
#define		GMSHADER_TEXTURES_SCROLL_S			".scroll_s"
#define		GMSHADER_TEXTURES_SCROLL_T			".scroll_t"
#define		GMSHADER_TEXTURES_SCALE_S			".scale_s"
#define		GMSHADER_TEXTURES_SCALE_T			".scale_t"

//光照
#define		GMSHADER_AMBIENT_LIGHTS				GMSHADER("ambients")
#define		GMSHADER_SPECULAR_LIGHTS			GMSHADER("speculars")
#define		GMSHADER_LIGHTS_LIGHTCOLOR			".lightColor"
#define		GMSHADER_LIGHTS_LIGHTPOSITION		".lightPosition"

//材质
#define		GMSHADER_MATERIAL_KA				GMSHADER("material.ka")
#define		GMSHADER_MATERIAL_KD				GMSHADER("material.kd")
#define		GMSHADER_MATERIAL_KS				GMSHADER("material.ks")
#define		GMSHADER_MATERIAL_SHININESS			GMSHADER("material.shininess")

//绘制调试
#define		GMSHADER_DEBUG_DRAW_NORMAL			GMSHADER("debug_draw_normal")

END_NS
#endif
