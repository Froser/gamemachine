#version 330

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
    vec3 worldNormalCoord;

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
    mat4 normalInverseMatrix = transpose(modelMatrix);
    mat4 normalWorldTransformMatrix = inverse(normalInverseMatrix);
    return normalWorldTransformMatrix;
}

_Coords calcCoords()
{
    _Coords coords;
    coords.worldCoord = GM_model_matrix * position;
    coords.modelViewCoord = GM_view_matrix * coords.worldCoord;
    coords.position = GM_projection_matrix * coords.modelViewCoord;

    mat4 normalTransform = calcNormalWorldTransformMatrix(GM_model_matrix);
    vec4 normalWorld = normalTransform * normal;
    coords.worldNormalCoord = normalize(normalWorld.xyz);

    coords.lightDirection = normalize(GM_light_position.xyz - coords.worldCoord.xyz);
    coords.viewDirection = normalize(GM_view_position.xyz - coords.worldCoord.xyz);
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
    float diffuse = dot(coords.lightDirection, coords.worldNormalCoord);
    diffuse = clamp(diffuse, 0.0f, 1.0f);
    return diffuse;
}

// 计算镜面反射率
float calcSpecular(_Coords coords, float shininess)
{
    vec3 halfVector = normalize(coords.lightDirection + coords.viewDirection);
    float specularPower = dot(halfVector, coords.worldNormalCoord);
    float specular = pow(specularPower, GM_light_shininess);
    specular = clamp(specular, 0.0f, 1.0f);

    return specular;
}

_LightFactors calcLightFactors(_Coords coords)
{
    _LightFactors lightFactors;
    lightFactors.diffuse = calcDiffuse(coords);

    float shininess = GM_light_shininess;
    lightFactors.specular = calcSpecular(coords, shininess);
    return lightFactors;
}

void main(void)
{
    coords = calcCoords();
    textureUVs = calcTexture(coords.viewDirection);
    lightFactors = calcLightFactors(coords);
    //if (lightFactors.diffuse <= 0.001)
        //return;
    gl_Position = coords.position;
}