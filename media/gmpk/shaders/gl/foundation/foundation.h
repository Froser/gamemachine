// 基本参数
uniform mat4 GM_ViewMatrix;
uniform mat4 GM_WorldMatrix;
uniform mat4 GM_ProjectionMatrix;
uniform mat4 GM_InverseTransposeModelMatrix;
uniform mat4 GM_InverseViewMatrix;

const int GM_MaxBones = 512;
uniform mat4 GM_Bones[GM_MaxBones];

// 类型变量
uniform int GM_shader_type;
uniform int GM_shader_proc;

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