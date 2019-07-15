// 基本参数
uniform mat4 GM_ViewMatrix;
uniform mat4 GM_WorldMatrix;
uniform mat4 GM_ProjectionMatrix;
uniform mat4 GM_InverseTransposeModelMatrix;
uniform mat4 GM_InverseViewMatrix;

const int GM_MaxBones = 128;
uniform mat4 GM_Bones[GM_MaxBones];
const int GM_NoAnimation = 0;
const int GM_SkeletalAnimation = 1;
const int GM_AffineAnimation = 2;
uniform int GM_UseAnimation = GM_NoAnimation;

// 类型变量
uniform int GM_shader_type;
uniform int GM_shader_proc;

const int GM_MaxCascadeLevel = 8;
struct GMShadowInfo
{
    int HasShadow;
    mat4 ShadowMatrix[GM_MaxCascadeLevel];
    float EndClip[GM_MaxCascadeLevel];
    int CurrentCascadeLevel;
    vec4 Position;
    sampler2D GM_ShadowMap;
    int ShadowMapWidth;
    int ShadowMapHeight;
    float BiasMin;
    float BiasMax;
    int CascadedShadowLevel;
    int ViewCascade;
    int PCFRows;
};

uniform GMShadowInfo GM_ShadowInfo;