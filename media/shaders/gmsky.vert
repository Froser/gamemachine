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

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;
uniform vec4 GM_view_position;

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

    // 视角方向
    STRUCT_MEMBER_OUT vec3 viewDirection;
END_STRUCT;
OUT(_Coords, coords);

BEGIN_STRUCT(_TextureUVs)
    // CubeMap贴图坐标
    STRUCT_MEMBER_OUT vec3 cubemapUV;
END_STRUCT;
OUT(_TextureUVs, textureUVs);

RET(_Coords) calcCoords()
{
    DIM(_Coords, coords);
    MEMBER(coords, worldCoord) = GM_model_matrix * position;
    MEMBER(coords, modelViewCoord) = GM_view_matrix * MEMBER(coords, worldCoord);
    MEMBER(coords, projectionCoord) = GM_projection_matrix * MEMBER(coords, modelViewCoord);
    MEMBER(coords, viewDirection) = normalize(GM_view_position.xyz - MEMBER(coords, worldCoord.xyz));
    RETURN(coords);
}

RET(_TextureUVs) calcTexture(vec3 viewDirection)
{
    DIM(_TextureUVs, uvs);
    MEMBER(uvs, cubemapUV) = -viewDirection;
    RETURN(uvs);
}

void main(void)
{
    ASSIGN(coords, calcCoords());
    ASSIGN(textureUVs, calcTexture(MEMBER(coords, viewDirection)));

    gl_Position = MEMBER(coords, projectionCoord);
}