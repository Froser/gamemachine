// 基本参数
uniform mat4 GM_shadow_matrix;
uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

// 类型变量
uniform int GM_shader_type;
uniform int GM_shader_proc;

// GM_shader_type
#define TYPE_MODEL2D			0
#define TYPE_MODEL3D			1
#define TYPE_PARTICLES			2
#define TYPE_GLYPH				3
#define TYPE_CUBEMAP			4

mat4 gm_removeTranslation(mat4 mat)
{
	mat4 r = mat4(
		vec4(mat[0].xyz, 0),
		vec4(mat[1].xyz, 0),
		vec4(mat[2].xyz, 0),
		vec4(0, 0, 0, 1)
	);
	return r;
}