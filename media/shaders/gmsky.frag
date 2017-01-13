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

// 环境立方体纹理（绘制天空时）
uniform samplerCube GM_cubemap_texture;
uniform vec4 GM_light_ambient;

BEGIN_STRUCT(_TextureUVs)
    // CubeMap贴图坐标
    STRUCT_MEMBER_IN vec3 cubemapUV;
END_STRUCT;
IN(_TextureUVs, textureUVs);

out vec4 frag_color;

void drawSky(vec3 cubemapUV)
{
    vec3 cubemapTextureColor = vec3(texture(GM_cubemap_texture, cubemapUV));
    frag_color = GM_light_ambient + vec4(cubemapTextureColor, 1.0f);
}

void main()
{
    // 如果存在立方体纹理，说明是环境（天空）的绘制，那么只考虑环境光，并将Ka全部设置为1
    drawSky(MEMBER(textureUVs, cubemapUV));
}
