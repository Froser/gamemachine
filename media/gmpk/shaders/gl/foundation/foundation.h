// 基本参数
uniform mat4 GM_ViewMatrix;
uniform mat4 GM_WorldMatrix;
uniform mat4 GM_ProjectionMatrix;
uniform mat4 GM_InverseTransposeModelMatrix;
uniform mat4 GM_InverseViewMatrix;

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
    sampler2D GM_ShadowMap;
    int ShadowMapWidth;
    int ShadowMapHeight;
    float BiasMin;
    float BiasMax;
};

uniform GMShadowInfo GM_ShadowInfo;

uniform int GM_TechniqueId;