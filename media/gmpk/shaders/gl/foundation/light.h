// 光照相关
#define MAX_LIGHT_COUNT 50
struct GM_Attenuation_t
{
    float Constant;
    float Linear;
    float Exp;
};
struct GM_light_t
{
    vec3 Color;
    vec3 Position;
    vec3 AmbientIntensity;
    vec3 DiffuseIntensity;
    float SpecularIntensity;
    GM_Attenuation_t Attenuation;
    int Type; //0: point, 1: directional, 2: spot

    // Directional light
    vec3 Direction;
};

uniform GM_light_t GM_lights[MAX_LIGHT_COUNT];
uniform int GM_LightCount = 0;

const int GM_IlluminationModel_None = 0;
const int GM_IlluminationModel_Phong = 1;
const int GM_IlluminationModel_CookTorranceBRDF = 2;
uniform int GM_IlluminationModel;

// 光源种类
const int GM_PointLight = 0;
const int GM_DirectionalLight = 1;
const int GM_Spotlight = 1;

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
    return color / (color + vec3(1,1,1));
}

// Phong光照实现
vec3 GMLight_PointLightAmbient(GM_light_t light)
{
    return max(light.Color * light.AmbientIntensity, 0);
}

vec3 GMLight_PointLightDiffuse(GM_light_t light, vec3 lightDirection_N, vec3 normal_N)
{
    float diffuseFactor = dot(lightDirection_N, normal_N);
    return max(diffuseFactor * light.Color * light.DiffuseIntensity, 0);
}

vec3 GMLight_PointLightSpecular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
    vec3 R = normalize(reflect(-lightDirection_N, normal_N));
    float theta = max(dot(eyeDirection_N, R), 0);
    float specularFactor = (theta == 0 && shininess == 0) ? 0 : pow(theta, shininess);
    return specularFactor * light.Color * light.SpecularIntensity;
}

// 代理方法
vec3 GMLight_GetDirection_eye_N(GM_light_t light, vec3 eyeDirection_eye)
{
    if (light.Type == GM_PointLight)
    {
        vec3 lightPosition_eye = (GM_ViewMatrix * vec4(light.Position, 1)).xyz;
        vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
        return lightDirection_eye_N;
    }
    else if (light.Type == GM_DirectionalLight)
    {
        return mat3(GM_ViewMatrix) * -light.Direction;
    }
}

vec3 GMLight_Ambient(GM_light_t light)
{
    if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
        return GMLight_PointLightAmbient(light);

    return GMLight_PointLightAmbient(light);
}

vec3 GMLight_Diffuse(GM_light_t light, vec3 lightDirection_N, vec3 normal_N)
{
    if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
        return GMLight_PointLightDiffuse(light, lightDirection_N, normal_N);

    return GMLight_PointLightDiffuse(light, lightDirection_N, normal_N);
}

vec3 GMLight_Specular(GM_light_t light, vec3 lightDirection_N, vec3 eyeDirection_N, vec3 normal_N, float shininess)
{
    if (light.Type == GM_PointLight || light.Type == GM_DirectionalLight)
        return GMLight_PointLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);

    return GMLight_PointLightSpecular(light, lightDirection_N, eyeDirection_N, normal_N, shininess);
}


/////////////////////////////////////////////////////////////////////
// 基本光照流程
struct GMTangentSpace
{
    vec3 Normal_Tangent_N;
    mat3 TBN;
};

bool GM_IsTangentSpaceInvalid(vec3 tangent, vec3 bitangent)
{
    // 返回是否无切线空间
    return length(tangent) < 0.01f && length(bitangent) < 0.01f;
}

GMTangentSpace GM_CalculateTangentSpaceRuntime(
        vec3 worldPos,
        vec2 texcoord,
        vec3 normal_World_N,
        sampler2D normalMap
    )
{
    GMTangentSpace tangentSpace;
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
    GMTangentSpace TangentSpace;  // 切线空间
    bool HasNormalMap;          // 是否有法线贴图
    vec3 AmbientLightmapTexture;
    vec3 DiffuseTexture;
    vec3 SpecularTexture;
    float Shininess;
    float Refractivity;
    vec3 AlbedoTexture;
    vec3 MetallicRoughnessAOTexture;
    vec3 F0;
    int IlluminationModel;
};

vec3 calculateRefractionByNormalWorld(vec3 worldPos, vec3 normal_world_N, float Refractivity)
{
    if (Refractivity == 0.f)
        return vec3(0, 0, 0);

    vec3 I = normalize(worldPos - GM_ViewPosition.xyz);
    vec3 R = refract(I, normal_world_N, Refractivity);
    return texture(GM_CubeMapTextureAttribute, vec3(R.x, R.y, R.z)).rgb;
}

vec3 calculateRefractionByNormalTangent(vec3 worldPos, GMTangentSpace tangentSpace, float Refractivity)
{
    if (Refractivity == 0.f)
        return vec3(0, 0, 0);
    
    // 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
    vec3 normal_world_N = normalize(mat3(GM_InverseViewMatrix) * transpose(tangentSpace.TBN) * tangentSpace.Normal_Tangent_N);
    return calculateRefractionByNormalWorld(worldPos, normal_world_N, Refractivity);
}

float calculateShadow(mat4 shadowMatrix, vec4 worldPos, vec3 normal_N)
{
    if (GM_ShadowInfo.HasShadow == 0)
        return 1.0f;

    vec4 fragPos = shadowMatrix * worldPos;
    vec3 projCoords = fragPos.xyz / fragPos.w;
    if (projCoords.z > 1.0f)
        return 1.0f;
    projCoords = projCoords * 0.5f + 0.5f;

    float bias = (GM_ShadowInfo.BiasMin == GM_ShadowInfo.BiasMax) ? GM_ShadowInfo.BiasMin : max(GM_ShadowInfo.BiasMax * (1.0 - dot(normal_N, normalize(worldPos.xyz - GM_ShadowInfo.Position.xyz))), GM_ShadowInfo.BiasMin);
    float closestDepth = texture(GM_ShadowInfo.GM_ShadowMap, projCoords.xy).r;
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
    vec3 eyeDirection_eye = -(GM_ViewMatrix * vec4(vertex.WorldPos, 1)).xyz;
    vec3 eyeDirection_eye_N = normalize(eyeDirection_eye);

    // 计算漫反射和高光部分
    if (!vertex.HasNormalMap)
    {
        for (int i = 0; i < GM_LightCount; i++)
        {
            float distance = length(vertex.WorldPos - GM_lights[i].Position);
            float attenuation = GM_lights[i].Attenuation.Constant + 
                                GM_lights[i].Attenuation.Linear * distance +
                                GM_lights[i].Attenuation.Exp * distance * distance;

            vec3 lightDirection_eye_N = GMLight_GetDirection_eye_N(GM_lights[i], eyeDirection_eye);
            ambientLight += GMLight_Ambient(GM_lights[i]) / attenuation;
            diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_eye_N, vertex.Normal_Eye_N) / attenuation;
            specularLight += GMLight_Specular(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, vertex.Normal_Eye_N, vertex.Shininess) / attenuation;
            refractionLight += calculateRefractionByNormalWorld(vertex.WorldPos, vertex.Normal_World_N, vertex.Refractivity);
        }
    }
    else
    {
        for (int i = 0; i < GM_LightCount; i++)
        {
            float distance = length(vertex.WorldPos - GM_lights[i].Position);
            float attenuation = GM_lights[i].Attenuation.Constant + 
                                GM_lights[i].Attenuation.Linear * distance +
                                GM_lights[i].Attenuation.Exp * distance * distance;

            vec3 lightPosition_eye = (GM_ViewMatrix * vec4(GM_lights[i].Position, 1)).xyz;
            vec3 lightDirection_eye_N = GMLight_GetDirection_eye_N(GM_lights[i], eyeDirection_eye);
            vec3 lightDirection_tangent_N = normalize(vertex.TangentSpace.TBN * lightDirection_eye_N);
            vec3 eyeDirection_tangent_N = normalize(vertex.TangentSpace.TBN * eyeDirection_eye_N);

            ambientLight += GMLight_Ambient(GM_lights[i]) / attenuation;
            diffuseLight += GMLight_Diffuse(GM_lights[i], lightDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N) / attenuation;
            specularLight += GMLight_Specular(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, vertex.TangentSpace.Normal_Tangent_N, vertex.Shininess) / attenuation;
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
    vec3 viewDirection_N = normalize(GM_ViewPosition.rgb - vertex.WorldPos);
    // 换算回世界空间
    vec3 normal_World_N = normalize(transpose(vertex.TangentSpace.TBN) * vertex.TangentSpace.Normal_Tangent_N);
    float metallic = vertex.MetallicRoughnessAOTexture.r;
    float roughness = vertex.MetallicRoughnessAOTexture.g;
    float ao = vertex.MetallicRoughnessAOTexture.b;
    vec3 F0 = mix(vertex.F0, vertex.AlbedoTexture, metallic);
    vec3 Lo = vec3(0, 0, 0);
    vec3 ambient = vec3(0, 0, 0);
    
    for (int i = 0; i < GM_LightCount; ++i)
    {
        if (GM_lights[i].Type == GM_PointLight)
        {
            // 只考虑直接光源
            ambient += GM_lights[i].Color * GM_lights[i].AmbientIntensity * vertex.AlbedoTexture * roughness;
            // 计算每束光辐射率
            vec3 L_N = normalize(GM_lights[i].Position - vertex.WorldPos);
            vec3 H_N = normalize(viewDirection_N + L_N);

            float distance = length(vertex.WorldPos - GM_lights[i].Position);
            float attenuation = GM_lights[i].Attenuation.Constant + 
                                GM_lights[i].Attenuation.Linear * distance +
                                GM_lights[i].Attenuation.Exp * distance * distance;
            vec3 radiance = GM_lights[i].Color * GM_lights[i].DiffuseIntensity * attenuation;

            // Cook-Torrance BRDF
            float NDF = GM_DistributionGGX(normal_World_N, H_N, roughness);
            float G = GM_GeometrySmith(normal_World_N, viewDirection_N, L_N, roughness);
            vec3 F = GM_FresnelSchlick(max(dot(H_N, viewDirection_N), 0.0f), F0);
            vec3 nominator = NDF * G * F;
            float denominator = 4 * max(dot(normal_World_N, viewDirection_N), 0.0) * max(dot(normal_World_N, L_N), 0.0) + 0.001; // 0.001 防止除0
            vec3 specular = nominator / denominator;

            vec3 Ks = F;
            vec3 Kd = vec3(1, 1, 1) - Ks;
            Kd *= 1.0f - metallic;

            float cosTheta = max(dot(normal_World_N, L_N), 0);
            Lo += (Kd * vertex.AlbedoTexture / PI + specular) * radiance * cosTheta;
        }
    }

    vec3 color = (ambient + Lo) * shadowFactor;
    color = GM_ReinhardToneMapping(color);
    color = GM_CalculateGammaCorrection(color);
    return vec4(color, 1);
}

vec4 PS_3D_CalculateColor(PS_3D_INPUT vertex)
{
    float factor_Shadow = calculateShadow(GM_ShadowInfo.ShadowMatrix, vec4(vertex.WorldPos, 1), vertex.Normal_World_N);
    switch (vertex.IlluminationModel)
    {
        case GM_IlluminationModel_None:
            discard;
        case GM_IlluminationModel_Phong:
            return GM_Phong_CalculateColor(vertex, factor_Shadow);
        case GM_IlluminationModel_CookTorranceBRDF:
            return GM_CookTorranceBRDF_CalculateColor(vertex, factor_Shadow);
    }
    return vec4(0, 0, 0, 0);
}