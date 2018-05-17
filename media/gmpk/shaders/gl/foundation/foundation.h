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

struct GMShadowInfo
{
    int HasShadow;
    mat4 ShadowMatrix;
    vec4 Position;
    sampler2D ShadowMap;
    int ShadowMapWidth;
    int ShadowMapHeight;
    float BiasMin;
    float BiasMax;
};

uniform GMShadowInfo GM_shadowInfo;