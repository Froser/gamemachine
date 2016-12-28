#ifndef __SHADER_CONSTAINS_H__
#define __SHADER_CONSTAINS_H__
#include "common.h"
BEGIN_NS

enum
{
	GMTEXTURE_SHADOW = GL_TEXTURE0,
	GMTEXTURE_AMBIENT = GL_TEXTURE1,
};

enum
{
	GMTEXTURE_SHADOW_INDEX = 0,
	GMTEXTURE_AMBIENT_INDEX
};

#define		GMSHADER_MODEL_MATRIX		"model_matrix"
#define		GMSHADER_SHADOW_MATRIX		"shadow_matrix"
#define		GMSHADER_PROJECTION_MATRIX	"projection_matrix"
#define		GMSHADER_VIEW_MATRIX		"view_matrix"
#define		GMSHADER_VIEW_POSITION		"view_position"
#define		GMSHADER_LIGHT_POSITION		"light_position"
#define		GMSHADER_LIGHT_KA			"light_ka"
#define		GMSHADER_LIGHT_AMBIENT		"light_ambient"
#define		GMSHADER_LIGHT_COLOR		"light_color"
#define		GMSHADER_LIGHT_KD			"light_kd"
#define		GMSHADER_LIGHT_KS			"light_ks"
#define		GMSHADER_LIGHT_SHININESS	"light_shininess"
#define		GMSHADER_DEPTH_TEXTURE		"depth_texture"
#define		GMSHADER_AMBIENT_TEXTURE	"ambient_texture"

END_NS
#endif