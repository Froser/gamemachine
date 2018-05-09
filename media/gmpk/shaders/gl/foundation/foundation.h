// 基本参数
uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;
uniform mat4 GM_inverse_transpose_model_matrix;
uniform mat4 GM_inverse_view_matrix;

// 类型变量
uniform int GM_shader_type;
uniform int GM_shader_proc;

subroutine void GM_TechniqueEntrance();
subroutine (GM_TechniqueEntrance) void GM_Noop() {}
subroutine uniform GM_TechniqueEntrance GM_techniqueEntrance;

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

struct GMShadowInfo
{
	int HasShadow;
	mat4 ShadowProjectionMatrix;
	mat4 ShadowViewMatrix;
	vec4 Position;
	sampler2D ShadowMap;
	int ShadowMapWidth;
	int ShadowMapHeight;
	float BiasMin;
	float BiasMax;
};

uniform GMShadowInfo GM_shadowInfo;