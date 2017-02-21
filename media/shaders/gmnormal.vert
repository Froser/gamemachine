#version 330

uniform mat4 GM_shadow_matrix;
uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;
layout (location = 4) in vec4 bitangent;
layout (location = 5) in vec2 lightmapuv;

out vec4 shadowCoord;
out vec4 _normal;
out vec2 _uv;
out vec4 _tangent;
out vec4 _bitangent;
out vec2 _lightmapuv;
out vec4 position_world;

// 由顶点变换矩阵计算法向量变换矩阵
mat4 calcNormalWorldTransformMatrix()
{
    mat4 normalInverseMatrix = inverse(GM_model_matrix);
    mat4 normalWorldTransformMatrix = transpose(normalInverseMatrix);
    return normalWorldTransformMatrix;
}

void calcCoords()
{
    position_world = GM_model_matrix * position;
    vec4 position_eye = GM_view_matrix * position_world;

    gl_Position = GM_projection_matrix * position_eye;

    shadowCoord = GM_shadow_matrix * position_world;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
}

void main(void)
{
    calcCoords();
}