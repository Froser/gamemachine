#version 410
#include "../foundation/foundation.h"
#include "../foundation/properties.h"
#include "../foundation/light.h"

in vec2 _uv;
out vec4 _frag_color;

uniform sampler2D deferred_light_pass_gPosition_Refractivity;
uniform sampler2D deferred_light_pass_gNormal;
uniform sampler2D deferred_light_pass_gTexAmbient;
uniform sampler2D deferred_light_pass_gTexDiffuse;
uniform sampler2D deferred_light_pass_gTangent_eye;
uniform sampler2D deferred_light_pass_gBitangent_eye;
uniform sampler2D deferred_light_pass_gNormalMap_bNormalMap;
uniform sampler2D deferred_light_pass_gKs_Shininess;

// [0, 1] -> [-1, 1]
vec3 textureToNormal(vec3 tex)
{
    return tex * 2.f - 1;
}

void main()
{
    PS_3D_INPUT vertex;
    vec4 positionRefractivity = texture(deferred_light_pass_gPosition_Refractivity, _uv);
    vertex.WorldPos = positionRefractivity.rgb;
    vertex.Refractivity = positionRefractivity.a;
    vertex.Normal_World_N = textureToNormal(texture(deferred_light_pass_gNormal, _uv).rgb);
    vertex.Normal_Eye_N = mat3(GM_view_matrix) * vertex.Normal_World_N;

    vec4 normalMapHasNormalMap = texture(deferred_light_pass_gNormalMap_bNormalMap, _uv);
    vertex.HasNormalMap = normalMapHasNormalMap.a > 0;

    TangentSpace tangentSpace;
    vec3 tangent_eye = textureToNormal(texture(deferred_light_pass_gTangent_eye, _uv).rgb);
    vec3 bitangent_eye = textureToNormal(texture(deferred_light_pass_gBitangent_eye, _uv).rgb);
    mat3 TBN = transpose(mat3(
        tangent_eye,
        bitangent_eye,
        vertex.Normal_Eye_N
    ));
    tangentSpace.Normal_Tangent_N = textureToNormal(normalMapHasNormalMap.rgb);
    tangentSpace.TBN = TBN;
    vertex.TangentSpace = tangentSpace;
    vertex.AmbientLightmapTexture = texture(deferred_light_pass_gTexAmbient, _uv).rgb;
    vertex.DiffuseTexture = texture(deferred_light_pass_gTexDiffuse, _uv).rgb;

    vec4 ksShininess = texture(deferred_light_pass_gKs_Shininess, _uv);
    vertex.SpecularTexture = ksShininess.rgb;
    vertex.Shininess = ksShininess.a;

    _frag_color = PS_3D_CalculateColor(vertex);
}
