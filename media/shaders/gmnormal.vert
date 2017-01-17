#version 330

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

uniform mat4 GM_shadow_matrix;
uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

uniform vec4 GM_light_position;
uniform vec4 GM_view_position;
uniform float GM_light_shininess;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;

BEGIN_STRUCT(_Coords)
    // 顶点世界坐标
    STRUCT_MEMBER_OUT vec4 worldCoord;

    // 顶点在视角变换后的坐标
    STRUCT_MEMBER_OUT vec4 modelViewCoord;

    // 投影后的最终坐标
    STRUCT_MEMBER_OUT vec4 projectionCoord;

    // 变换后的世界坐标标准法向量
    STRUCT_MEMBER_OUT vec3 worldNormalCoord;

    // 灯光照射方向（目前灯光最多数量为1）
    STRUCT_MEMBER_OUT vec3 lightDirection;

    // 视角方向
    STRUCT_MEMBER_OUT vec3 viewDirection;

    // 阴影坐标
    STRUCT_MEMBER_OUT vec4 shadowCoord;
END_STRUCT;
OUT(_Coords, coords);

BEGIN_STRUCT(_TextureUVs)
    // 环境光贴图坐标
    STRUCT_MEMBER_OUT vec2 textureUV;

    // CubeMap贴图坐标
    STRUCT_MEMBER_OUT vec3 cubemapUV;
END_STRUCT;
OUT(_TextureUVs, textureUVs);

BEGIN_STRUCT(_LightFactors)
    // 漫反射系数
    STRUCT_MEMBER_OUT float diffuse;

    // 镜面反射系数
    STRUCT_MEMBER_OUT float specular;
END_STRUCT;
OUT(_LightFactors, lightFactors);

// 由顶点变换矩阵计算法向量变换矩阵
mat4 calcNormalWorldTransformMatrix(mat4 modelMatrix)
{
    mat4 normalInverseMatrix = inverse(modelMatrix);
    mat4 normalWorldTransformMatrix = transpose(normalInverseMatrix);
    return normalWorldTransformMatrix;
}

RET(_Coords) calcCoords()
{
    DIM(_Coords, coords);
    MEMBER(coords, worldCoord) = GM_model_matrix * position;
    MEMBER(coords, modelViewCoord) = GM_view_matrix * MEMBER(coords, worldCoord);
    MEMBER(coords, projectionCoord) = GM_projection_matrix * MEMBER(coords, modelViewCoord);

    mat4 normalTransform = calcNormalWorldTransformMatrix(GM_model_matrix);
    vec4 normalWorld = normalTransform * normal;
    MEMBER(coords, worldNormalCoord) = normalize(normalWorld.xyz);

    MEMBER(coords, lightDirection) = normalize(GM_light_position.xyz - MEMBER(coords, worldCoord.xyz));
    MEMBER(coords, viewDirection) = normalize(GM_view_position.xyz - MEMBER(coords, worldCoord.xyz));
    MEMBER(coords, shadowCoord) = GM_shadow_matrix * MEMBER(coords, worldCoord);
    RETURN(coords);
}

RET(_TextureUVs) calcTexture(vec3 viewDirection)
{
    DIM(_TextureUVs, uvs);
    MEMBER(uvs, textureUV) = uv;
    MEMBER(uvs, cubemapUV) = -viewDirection;
    RETURN(uvs);
}

// 计算漫反射率
float calcDiffuse(vec3 lightDirection, vec3 worldNormalCoord)
{
    if (worldNormalCoord.x == 0 && worldNormalCoord.y == 0 && worldNormalCoord.z == 0)
        return 1;

    float diffuse = dot(lightDirection, worldNormalCoord);
    diffuse = clamp(diffuse, 0.0f, 1.0f);
    return diffuse;
}

// 计算镜面反射率
float calcSpecular(vec3 lightDirection, vec3 viewDirection, vec3 worldNormalCoord)
{
    vec3 halfVector = normalize(lightDirection + viewDirection);
    float specularPower = dot(halfVector, worldNormalCoord);
    float specular = pow(specularPower, GM_light_shininess);
    specular = clamp(specular, 0.0f, 1.0f);

    return specular;
}

RET(_LightFactors) calcLightFactors(vec3 lightDirection, vec3 viewDirection, vec3 worldNormalCoord)
{
    DIM(_LightFactors, lightFactors);
    MEMBER(lightFactors, diffuse) = calcDiffuse(lightDirection, worldNormalCoord);
    MEMBER(lightFactors, specular) = calcSpecular(lightDirection, viewDirection, worldNormalCoord);
    RETURN(lightFactors);
}

void main(void)
{
    ASSIGN(coords, calcCoords());
    ASSIGN(textureUVs, calcTexture(MEMBER(coords, viewDirection)));
    ASSIGN(lightFactors, calcLightFactors(
        MEMBER(coords, lightDirection),
        MEMBER(coords, viewDirection),
        MEMBER(coords, worldNormalCoord)
        ));

    gl_Position = MEMBER(coords, projectionCoord);
}