#version 330 core

// 这是一套非常恶心的宏定义，是用于兼容不同机器对glsl的支持
// 有些显卡不支持结构体，因此我用一套宏来进行区分
#define STRUCT_SUPPORT 0

// 定义一个结构体
#if STRUCT_SUPPORT
#define BEGIN_STRUCT(name) struct name {
#else
#define BEGIN_STRUCT(name)
#endif

// 定义结构体成员
#if STRUCT_SUPPORT
#define STRUCT_MEMBER_OUT
#else
#define STRUCT_MEMBER_OUT out
#endif

// 定义结构体成员
#if STRUCT_SUPPORT
#define STRUCT_MEMBER_IN
#else
#define STRUCT_MEMBER_IN in
#endif

// 结束一个结构体
#if STRUCT_SUPPORT
#define END_STRUCT }
#else
#define END_STRUCT
#endif

// 实例化一个结构体
#if STRUCT_SUPPORT
#define DIM(type, name) type name
#else
#define DIM(type, name)
#endif

// 取结构体成员
#if STRUCT_SUPPORT
#define MEMBER(name, value) name.value
#else
#define MEMBER(name, value) value
#endif

// 结构体返回类型
#if STRUCT_SUPPORT
#define RET(type) type
#else
#define RET(type) void
#endif

// 返回成员
#if STRUCT_SUPPORT
#define RETURN(name) return name
#else
#define RETURN(name)
#endif

// 定义out结构体变量
#if STRUCT_SUPPORT
#define OUT(type, name) out type name
#else
#define OUT(type, name)
#endif

// 定义out结构体变量
#if STRUCT_SUPPORT
#define IN(type, name) in type name
#else
#define IN(type, name)
#endif

// 结构体赋值
#if STRUCT_SUPPORT
#define ASSIGN(var, value) var = value
#else
#define ASSIGN(var, value) value
#endif
/////////////////////////////////////////////////////
// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

// 受环境系数影响的纹理 (Ka)
uniform sampler2D GM_ambient_texture;
uniform int GM_ambient_texture_switch = 0;

// 受漫反射系数影响的纹理 (kd)
uniform sampler2D GM_diffuse_texture;
uniform int GM_diffuse_texture_switch = 0;

// 环境立方体纹理（反射天空时）
uniform samplerCube GM_reflection_cubemap_texture;
uniform int GM_reflection_cubemap_texture_switch = 0;

uniform vec4 GM_light_ambient;
uniform vec4 GM_light_ka;
uniform vec4 GM_light_specular;
uniform vec4 GM_light_kd;
uniform vec4 GM_light_ks;
uniform vec4 GM_light_ke;

BEGIN_STRUCT(_Coords)
    // 顶点世界坐标
    STRUCT_MEMBER_IN vec4 worldCoord;

    // 顶点在视角变换后的坐标
    STRUCT_MEMBER_IN vec4 modelViewCoord;

    // 投影后的最终坐标
    STRUCT_MEMBER_IN vec4 projectionCoord;

    // 变换后的世界坐标标准法向量
    STRUCT_MEMBER_IN vec3 worldNormalCoord;

    // 灯光照射方向（目前灯光最多数量为1）
    STRUCT_MEMBER_IN vec3 lightDirection;

    // 视角方向
    STRUCT_MEMBER_IN vec3 viewDirection;

    // 阴影坐标
    STRUCT_MEMBER_IN vec4 shadowCoord;
END_STRUCT;
IN(_Coords, coords);

BEGIN_STRUCT(_TextureUVs)
    // 环境光贴图坐标
    STRUCT_MEMBER_IN vec2 textureUV;

    // CubeMap贴图坐标
    STRUCT_MEMBER_IN vec3 cubemapUV;
END_STRUCT;
IN(_TextureUVs, textureUVs);

BEGIN_STRUCT(_LightFactors)
    // 漫反射系数
    STRUCT_MEMBER_IN float diffuse;

    // 镜面反射系数
    STRUCT_MEMBER_IN float specular;
END_STRUCT;
IN(_LightFactors, lightFactors);

out vec4 frag_color;

float calcuateShadeFactor(vec4 shadowCoord)
{
    if (GM_shadow_texture_switch == 0)
        return 1;

    float shadeFactor = 0.0;
    shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(-1, -1));
    shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(1, -1));
    shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(-1, 1));
    shadeFactor += textureProjOffset(GM_shadow_texture, shadowCoord, ivec2(1, 1));
    shadeFactor /= 4;

    return shadeFactor;
}

float shadeFactorFactor(float shadeFactor)
{
    return min(shadeFactor + 0.3, 1);
}

void drawObject()
{
    // 计算阴影系数
    float shadeFactor = shadeFactorFactor(calcuateShadeFactor(MEMBER(coords, shadowCoord)));

    // 环境光
    vec3 ambientLight = vec3(0);

    // 计算点光源（漫反射、Kd和镜面反射）
    vec3 diffuseLight = MEMBER(lightFactors, diffuse) * vec3(GM_light_specular);
    vec3 diffuseTextureColor = GM_diffuse_texture_switch == 1 ? vec3(texture(GM_diffuse_texture, MEMBER(textureUVs,textureUV))) : vec3(0);
    diffuseLight += diffuseTextureColor;
    diffuseLight *= vec3(GM_light_kd);

    vec3 specularLight = MEMBER(lightFactors, specular) * vec3(GM_light_specular) * vec3(GM_light_ks);

    // 根据环境反射度来反射天空盒（如果有的话）
    if (GM_reflection_cubemap_texture_switch == 1)
    {
        vec3 reflectionCoord = reflect(-MEMBER(coords, viewDirection), MEMBER(coords, worldNormalCoord).xyz);
        // 乘以shadeFactor是因为阴影会遮挡反射光
        if (GM_light_ke.x > 0 && GM_light_ke.y > 0 && GM_light_ke.z > 0)
        {
            vec3 color_from_reflection = shadeFactor * (texture(GM_reflection_cubemap_texture, reflectionCoord).rgb * vec3(GM_light_ke));
            ambientLight += color_from_reflection;
        }
    }

    // 计算环境光和Ka贴图
    vec3 ambientTextureColor = GM_ambient_texture_switch == 1 ? vec3(texture(GM_ambient_texture, MEMBER(textureUVs,textureUV))) : vec3(0);
    ambientLight += GM_light_ambient.xyz + shadeFactor * ambientTextureColor;
    ambientLight *= vec3(GM_light_ka);

    // 最终结果
    vec3 color = ambientLight + shadeFactor * (diffuseLight + specularLight);
    frag_color = vec4(color, 1.0f);
}

void main()
{
    drawObject();
}
