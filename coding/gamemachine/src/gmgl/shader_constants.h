#ifndef __SHADER_CONSTAINS_H__
#define __SHADER_CONSTAINS_H__
#include "common.h"
BEGIN_NS

#define		GMSHADER(memberName)			"GM_" memberName

#define		GMSHADER_MODEL_MATRIX				GMSHADER("model_matrix")
#define		GMSHADER_SHADOW_MATRIX				GMSHADER("shadow_matrix")
#define		GMSHADER_PROJECTION_MATRIX			GMSHADER("projection_matrix")
#define		GMSHADER_VIEW_MATRIX				GMSHADER("view_matrix")
#define		GMSHADER_VIEW_POSITION				GMSHADER("view_position")
#define		GMSHADER_LIGHT_POSITION				GMSHADER("light_position")
#define		GMSHADER_LIGHT_KA					GMSHADER("light_ka")
#define		GMSHADER_LIGHT_AMBIENT				GMSHADER("light_ambient")
#define		GMSHADER_LIGHT_POWER				GMSHADER("light_power")
#define		GMSHADER_LIGHT_KD					GMSHADER("light_kd")
#define		GMSHADER_LIGHT_KS					GMSHADER("light_ks")
#define		GMSHADER_LIGHT_SHININESS			GMSHADER("light_shininess")
#define		GMSHADER_AMBIENT_TEXTURES			GMSHADER("ambient_textures")
#define		GMSHADER_DIFFUSE_TEXTURE			GMSHADER("diffuse_texture")
#define		GMSHADER_NORMAL_MAPPING_TEXTURE		GMSHADER("normal_mapping_texture")
#define		GMSHADER_LIGHTMAP_TEXTURE			GMSHADER("lightmap_texture")

//绘制调试
#define		GMSHADER_DEBUG_DRAW_NORMAL			GMSHADER("debug_draw_normal")

END_NS
#endif
