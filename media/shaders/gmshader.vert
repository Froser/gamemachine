#version 400

uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;
uniform mat4 GM_shadow_matrix;

uniform vec4 light_position;
uniform vec4 view_position;
uniform float light_shininess;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;

out float diffuse;
out float specular;

struct _Coords
{
    // 顶点世界坐标
    vec4 worldCoord;

    // 顶点在视角变换后的坐标
    vec4 modelViewCoord;

    // 投影后的最终坐标
    vec4 position;

    // 变换后的世界坐标标准法向量
    vec4 worldNormalCoord;

    // 灯光照射方向（目前灯光最多数量为1）
    vec3 lightDirection;

    // 视角方向
    vec3 viewDirection;

    // 阴影坐标
    vec4 shadowCoord;
};

struct _TextureUVs
{
    // 环境光贴图坐标
    vec2 ambientUV;

    // CubeMap贴图坐标
    vec3 cubemapUV;
};

struct _LightFactors
{
    // 漫反射系数
    float diffuse;

    // 镜面反射系数
    float specular;
};

out _Coords coords;
out _TextureUVs textureUVs;
out _LightFactors lightFactors;

// 由顶点变换矩阵计算法向量变换矩阵
mat4 calcNormalWorldTransformMatrix(mat4 modelMatrix)
{
    mat4 normalInverseMatrix = inverse(modelMatrix);
    mat4 normalWorldTransformMatrix = transpose(normalInverseMatrix);
    return normalWorldTransformMatrix;
}

_Coords calcCoords()
{
    _Coords coords;
    coords.worldCoord = GM_model_matrix * position;
    coords.modelViewCoord = GM_view_matrix * coords.worldCoord;
    coords.position = GM_projection_matrix * coords.modelViewCoord;
    coords.worldNormalCoord = normalize(calcNormalWorldTransformMatrix(GM_model_matrix) * normal);
    coords.lightDirection = normalize(light_position.xyz - coords.worldCoord.xyz);
    coords.viewDirection = normalize(view_position.xyz - coords.worldCoord.xyz);
    coords.shadowCoord = GM_shadow_matrix * coords.worldCoord;
    return coords;
}

_TextureUVs calcTexture(vec3 viewDirection)
{
    _TextureUVs uvs;
    uvs.ambientUV = uv;
    uvs.cubemapUV = -viewDirection;
    return uvs;
}

// 计算漫反射率
float calcDiffuse(_Coords coords)
{
    return dot(coords.lightDirection, vec3(coords.worldNormalCoord));
}

// 计算镜面反射率
float calcSpecular(_Coords coords, float shininess)
{
    vec3 halfVector = 0.5 * (coords.lightDirection + coords.viewDirection);
    float specularPower = dot(halfVector, coords.worldNormalCoord.xyz);
    return pow(specularPower, shininess);
}

_LightFactors calcLightFactors(_Coords coords)
{
    _LightFactors lightFactors;
    lightFactors.diffuse = calcDiffuse(coords);
    lightFactors.specular = calcSpecular(coords, light_shininess);
    return lightFactors;
}

void main(void)
{
    coords = calcCoords();
    textureUVs = calcTexture(coords.viewDirection);
    lightFactors = calcLightFactors(coords);
    gl_Position = coords.position;
}