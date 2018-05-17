// 光照相关
#define MAX_LIGHT_COUNT 50
struct GM_light_t
{
    vec3 LightColor;
    vec3 LightPosition;
    int LightType; //0表示环境光，1表示直接光
};

uniform GM_light_t GM_lights[MAX_LIGHT_COUNT];
uniform int GM_lightCount = 0;

const int GM_IlluminationModel_Phong = 0;
const int GM_IlluminationModel_CookTorranceBRDF = 1;
uniform int GM_IlluminationModel;

// 光源种类
const int GM_AmbientLight = 0;
const int GM_DirectLight = 1;

// Gamma校正
uniform int GM_GammaCorrection;
uniform float GM_Gamma;
uniform float GM_GammaInv;
vec3 GM_CalculateGammaCorrection(vec3 factor)
{
    return pow(factor, vec3(GM_GammaInv, GM_GammaInv, GM_GammaInv));
}

vec3 GM_CalculateGammaCorrectionIfNecessary(vec3 factor)
{
    if (GM_GammaCorrection == 0)
        return factor;

    return GM_CalculateGammaCorrection(factor);
}

// ToneMapping
vec3 GM_ReinhardToneMapping(vec3 color)
{
    return vec4(color / (color + vec3(1,1,1)), 1);
}

// Phong光照实现
vec3 GMLight_AmbientLightAmbient(GM_light_t light)
{
    return light.LightColor;
}

vec3 GMLight_AmbientLightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
    return vec3(0, 0, 0);
}

vec3 GMLight_AmbientLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
    return vec3(0, 0, 0);
}

vec3 GMLight_DirectLightAmbient(GM_light_t light)
{
    return vec3(0, 0, 0);
}

vec3 GMLight_DirectLightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
    float diffuseFactor = dot(lightDirection_N, normal_N);
    diffuseFactor = diffuseFactor;
    return max(diffuseFactor * light.LightColor, 0);
}

vec3 GMLight_DirectLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
    vec3 R = normalize(reflect(-lightDirection_N, normal_N));
    float theta = max(dot(eyeDirection_N, R), 0);
    float specularFactor = (theta == 0 && shininess == 0) ? 0 : pow(theta, shininess);
    specularFactor = specularFactor;
    return specularFactor * light.LightColor;
}

// 代理方法
vec3 GMLight_Ambient(GM_light_t light)
{
    if (light.LightType == GM_AmbientLight)
        return GMLight_AmbientLightAmbient(light);
    return GMLight_DirectLightAmbient(light);
}

vec3 GMLight_Diffuse(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N)
{
    if (light.LightType == GM_AmbientLight)
        return GMLight_AmbientLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
    return GMLight_DirectLightDiffuse(light, lightDirection_N, eyeDirection_N, normal_N);
}

vec3 GMLight_Specular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
    if (light.LightType == GM_AmbientLight)
        return GMLight_AmbientLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);
    return GMLight_DirectLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);
}


/////////////////////////////////////////////////////////////////////
// 基本光照流程
struct TangentSpace
{
    vec3 Normal_Tangent_N;
    mat3 TBN;
};

bool isTangentSpaceInvalid(vec3 tangent, vec3 bitangent)
{
    // 返回是否有切线空间
    return length(tangent) < 0.01f && length(bitangent) < 0.01f;
}

TangentSpace calculateTangentSpaceRuntime(
        vec3 worldPos,
        vec2 texcoord,
        vec3 normal_World_N,
        sampler2D normalMap
    )
{
    TangentSpace tangentSpace;
    tangentSpace.Normal_Tangent_N = texture(normalMap, texcoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(worldPos);
    vec3 Q2  = dFdy(worldPos);
    vec2 st1 = dFdx(texcoord);
    vec2 st2 = dFdy(texcoord);

    vec3 N   = normalize(normal_World_N);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    tangentSpace.TBN = transpose(mat3(T, B, N)); //TBN是世界空间到切线空间的变换矩阵
    return tangentSpace;
}

struct PS_3D_INPUT
{
    vec3 WorldPos;            // 世界坐标
    vec3 Normal_World_N;      // 世界法线
    vec3 Normal_Eye_N;        // 眼睛空间法向量
    TangentSpace TangentSpace;  // 切线空间
    bool HasNormalMap;          // 是否有法线贴图
    vec3 AmbientLightmapTexture;
    vec3 DiffuseTexture;
    vec3 SpecularTexture;
    vec3 AlbedoTexture;
    vec3 MetallicRoughnessAOTexture;
    float Shininess;
    float Refractivity;
};

vec3 calculateRefractionByNormalWorld(vec3 worldPos, vec3 normal_world_N, float refractivity)
{
    if (refractivity == 0.f)
        return vec3(0, 0, 0);

    vec3 I = normalize(worldPos - GM_view_position.xyz);
    vec3 R = refract(I, normal_world_N, refractivity);
    return texture(GM_cubemap_texture, vec3(R.x, R.y, R.z)).rgb;
}

vec3 calculateRefractionByNormalTangent(vec3 worldPos, TangentSpace tangentSpace, float refractivity)
{
    if (refractivity == 0.f)
        return vec3(0, 0, 0);
    
    // 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
    vec3 normal_world_N = normalize(mat3(GM_inverse_view_matrix) * transpose(tangentSpace.TBN) * tangentSpace.Normal_Tangent_N);
    return calculateRefractionByNormalWorld(worldPos, normal_world_N, refractivity);
}

float calculateShadow(mat4 shadowMatrix, vec4 worldPos, vec3 normal_N)
{
    if (GM_shadowInfo.HasShadow == 0)
        return 1.0f;

    vec4 fragPos = shadowMatrix * worldPos;
    vec3 projCoords = fragPos.xyz / fragPos.w;
    if (projCoords.z > 1.0f)
        return 1.0f;
    projCoords = projCoords * 0.5f + 0.5f;

    float bias = (GM_shadowInfo.BiasMin == GM_shadowInfo.BiasMax) ? GM_shadowInfo.BiasMin : max(GM_shadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - GM_shadowInfo.Position.xyz))), GM_shadowInfo.BiasMin);
    float closestDepth = texture(GM_shadowInfo.ShadowMap, projCoords.xy).r;
    return projCoords.z - bias > closestDepth ? 0.f : 1.f;
}

vec4 GM_Phong_CalculateColor(PS_3D_INPUT vertex, float shadowFactor)
{
    vec3 ambientLight = vec3(0, 0, 0);
    vec3 diffuseLight = vec3(0, 0, 0);
    vec3 specularLight = vec3(0, 0, 0);
    vertex.AmbientLightmapTexture = max(vertex.AmbientLightmapTexture, vec3(0, 0, 0));
    vertex.DiffuseTexture = max(vertex.DiffuseTexture, vec3(0, 0, 0));
    vertex.SpecularTexture = max(vertex.SpecularTexture, vec3(0, 0, 0));

    vec3 refractionLight = vec3(0, 0, 0);
    vec3 eyeDirection_eye = -(GM_view_matrix * vec4(vertex.WorldPos, 1)).xyz;
    vec3 eyeDirection_eye_N = normalize(eyeDirection_eye);

    // 计算漫反射和高光部分
    if (!vertex.HasNormalMap)
    {
        for (int i = 0; i < GM_lightCount; i++)
        {
            vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].LightPosition, 1)).xyz;
            vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
            ambientLight += GMLight_Ambient(GM_lights[i]);
            diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N);
            specularLight += GMLight_Specular(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N, vertex.Shininess);
            if (GM_lights[i].LightType == GM_AmbientLight)
                refractionLight += calculateRefractionByNormalWorld(vertex.WorldPos, vertex.Normal_World_N, vertex.Refractivity);
        }
    }
    else
    {
        for (int i = 0; i < GM_lightCount; i++)
        {
            vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].LightPosition, 1)).xyz;
            vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
            vec3 lightDirection_tangent_N = normalize(vertex.TangentSpace.TBN * lightDirection_eye_N);
            vec3 eyeDirection_tangent_N = normalize(vertex.TangentSpace.TBN * eyeDirection_eye_N);

            ambientLight += GMLight_Ambient(GM_lights[i]);
            diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N);
            specularLight += GMLight_Specular(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N, vertex.Shininess);
            if (GM_lights[i].LightType == GM_AmbientLight)
                refractionLight += calculateRefractionByNormalTangent(vertex.WorldPos, vertex.TangentSpace, vertex.Refractivity);
        }
    }
    vec3 finalColor =   vertex.AmbientLightmapTexture * GM_CalculateGammaCorrectionIfNecessary(ambientLight) +
                        vertex.DiffuseTexture * GM_CalculateGammaCorrectionIfNecessary(diffuseLight) * shadowFactor +
                        specularLight * GM_CalculateGammaCorrectionIfNecessary(vertex.SpecularTexture) * shadowFactor +
                        refractionLight;
    return vec4(finalColor, 1);
}

// Cook Torrance BRDF
const float PI = 3.1415927f;
float GM_DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GM_GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GM_GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GM_GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GM_GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 GM_FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec4 GM_CookTorranceBRDF_CalculateColor(PS_3D_INPUT vertex, float shadowFactor)
{
    vec3 viewDirection_N = normalize(GM_view_position.rgb - vertex.WorldPos);

    // 换算回世界空间
    vec3 normal_World_N = normalize(transpose(vertex.TangentSpace.TBN) * vertex.TangentSpace.Normal_Tangent_N);
    float metallic = vertex.MetallicRoughnessAOTexture.r;
    float roughness = vertex.MetallicRoughnessAOTexture.g;
    float ao = vertex.MetallicRoughnessAOTexture.b;
    vec3 F0 = mix(GM_material.f0, vertex.AlbedoTexture, metallic);
    vec3 Lo = vec3(0, 0, 0);
    vec3 ambient = vec3(0, 0, 0);
    for (int i = 0; i < GM_lightCount; ++i)
    {
        // 只考虑直接光源
        if (GM_lights[i].LightType == GM_AmbientLight)
        {
            //ambient += GM_lights[i].LightColor * vertex.AlbedoTexture * roughness;
        }
        else if (GM_lights[i].LightType == GM_DirectLight)
        {
            // 计算每束光辐射率
            vec3 L_N = normalize(GM_lights[i].LightPosition - vertex.WorldPos);
            vec3 H_N = normalize(viewDirection_N + L_N);
            float attenuation = 1.0f; //先不计算衰减
            vec3 radiance = GM_lights[i].LightColor * attenuation;

            // Cook-Torrance BRDF
            float NDF = GM_DistributionGGX(normal_World_N, H_N, roughness);
            float G = GM_GeometrySmith(normal_World_N, viewDirection_N, L_N, roughness);
            vec3 F = GM_FresnelSchlick(max(dot(H_N, viewDirection_N), 0.0f), F0);
            vec3 nominator = NDF * G * F;
            float denominator = 4 * max(dot(normal_World_N, viewDirection_N), 0.0) * max(dot(normal_World_N, L_N), 0.0) + 0.001; // 0.001 防止除0
            vec3 specular = nominator / denominator;

            vec3 ks = F;
            vec3 kd = vec3(1, 1, 1) - ks;
            kd *= 1.0f - metallic;

            float cosTheta = max(dot(normal_World_N, L_N), 0);
            Lo += (kd * vertex.AlbedoTexture / PI + specular) * radiance * cosTheta;
        }
    }

    vec3 color = (ambient + Lo) * shadowFactor;
    color = GM_ReinhardToneMapping(color);
    color = GM_CalculateGammaCorrection(color);
    return vec4(color, 1);
}

vec4 PS_3D_CalculateColor(PS_3D_INPUT vertex)
{
    float factor_Shadow = calculateShadow(GM_shadowInfo.ShadowMatrix, vec4(vertex.WorldPos, 1), vertex.Normal_World_N);
    switch (GM_IlluminationModel)
    {
        case GM_IlluminationModel_Phong:
            return GM_Phong_CalculateColor(vertex, factor_Shadow);
        case GM_IlluminationModel_CookTorranceBRDF:
            return GM_CookTorranceBRDF_CalculateColor(vertex, factor_Shadow);
    }
    return vec4(0, 0, 0, 0);
}