#version 330 core

in vec4 position_world;
in vec4 _normal;
in vec2 _uv;
in vec4 _tangent;
in vec4 _bitangent;
in vec2 _lightmapuv;
in vec4 shadowCoord;

// 阴影纹理
uniform sampler2DShadow GM_shadow_texture;
uniform int GM_shadow_texture_switch = 0;

// 受环境系数影响的纹理 (Ka)
uniform sampler2D GM_ambient_texture;
uniform int GM_ambient_texture_switch = 0;
uniform sampler2D GM_ambient_texture_2;
uniform int GM_ambient_texture_2_switch = 0;
uniform sampler2D GM_ambient_texture_3;
uniform int GM_ambient_texture_3_switch = 0;

// 受漫反射系数影响的纹理 (kd)
uniform sampler2D GM_diffuse_texture;
uniform int GM_diffuse_texture_switch = 0;

// 环境立方体纹理（反射天空时）
uniform samplerCube GM_reflection_cubemap_texture;
uniform int GM_reflection_cubemap_texture_switch = 0;

// 法线贴图纹理
uniform sampler2D GM_normal_mapping_texture;
uniform int GM_normal_mapping_texture_switch = 0;

// 光照贴图纹理
uniform sampler2D GM_lightmap_texture;
uniform int GM_lightmap_texture_switch = 0;

uniform mat4 GM_view_matrix;
uniform vec4 GM_light_ambient;
uniform vec4 GM_light_ka;
uniform vec4 GM_light_specular;
uniform vec4 GM_light_kd;
uniform vec4 GM_light_ks;
uniform vec4 GM_light_ke;

uniform vec4 GM_light_position;
uniform float GM_light_shininess;
uniform mat4 GM_model_matrix;
uniform vec4 GM_view_position;

vec3 DEBUG;

// 漫反射系数
float g_diffuse;
// 镜面反射系数
float g_specular;
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

void calcDiffuseAndSpecular(vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
    //diffuse:
    {
        g_diffuse = dot(lightDirection, normal);
        g_diffuse = clamp(g_diffuse, 0.0f, 1.0f);
    }

    // specular
    {
        vec3 E = normalize(eyeDirection);
        vec3 R = reflect(-E, normal);
        float theta = dot(E, R);
        g_specular = pow(theta, GM_light_shininess);
        g_specular = clamp(g_specular, 0.0f, 1.0f);
    }
}

// 由顶点变换矩阵计算法向量变换矩阵
mat4 normalTransform = transpose(inverse(GM_model_matrix));
mat4 normalEyeTransform = GM_view_matrix * normalTransform;

void calcLights()
{
    vec4 position_eye = GM_view_matrix * position_world;
    vec3 eyeDirection_eye = normalize(vec3(0,0,0) - position_eye.xyz);
    vec3 lightPosition_eye = (GM_view_matrix * GM_light_position).xyz;
    vec3 lightDirection_eye = normalize(lightPosition_eye + eyeDirection_eye);
    vec3 normal_eye = normalize((normalEyeTransform * _normal).xyz);

    if (GM_normal_mapping_texture_switch == 0)
    {
        calcDiffuseAndSpecular(lightDirection_eye, eyeDirection_eye, normal_eye);
    }
    else
    {
        vec3 tangent_eye = normalize((normalEyeTransform * _tangent).xyz);
        vec3 bitangent_eye = normalize((normalEyeTransform * _bitangent).xyz);
        mat3 TBN = transpose(mat3(
            tangent_eye,
            bitangent_eye,
            normal_eye.xyz
        ));

        vec3 lightDirection_tangent = normalize(TBN * lightDirection_eye);
        vec3 eyeDirection_tangent = normalize(TBN * eyeDirection_eye);
        vec3 normal_tangent = normalize(texture(GM_normal_mapping_texture, _uv).rgb * 2.0 - 1.0);

        calcDiffuseAndSpecular(lightDirection_tangent, eyeDirection_tangent, normal_tangent);
        //DEBUG.rgb = _tangent.xyz;
    }
}

void drawObject()
{
    calcLights();

    // 计算阴影系数
    float shadeFactor = shadeFactorFactor(calcuateShadeFactor(shadowCoord));

    // 环境光
    vec3 ambientLight = vec3(0);

    // 计算点光源（漫反射、Kd和镜面反射）
    vec3 diffuseTextureColor = GM_diffuse_texture_switch == 1 ? vec3(texture(GM_diffuse_texture, _uv)) : vec3(1);
    vec3 diffuseLight = g_diffuse * vec3(GM_light_kd) * diffuseTextureColor;
    diffuseLight *= vec3(GM_light_specular);

    vec3 specularLight = g_specular * vec3(GM_light_specular) * vec3(GM_light_ks);

    // 计算环境光和Ka贴图
    //ambientTextureColor = GM_ambient_texture_switch == 1 ? vec3(texture(GM_ambient_texture, _uv)) : vec3(1);
    vec3  ambientTextureColor = GM_ambient_texture_2_switch == 1 ? vec3(texture(GM_ambient_texture_2, _uv)) : vec3(1);
    //ambientTextureColor *= GM_ambient_texture_3_switch == 1 ? vec3(texture(GM_ambient_texture_3, _uv)) : vec3(1);
    ambientTextureColor *= GM_lightmap_texture_switch == 1 ? vec3(texture(GM_lightmap_texture, _lightmapuv)) : vec3(1);
    ambientLight += GM_light_ka.xyz * shadeFactor * ambientTextureColor;
    ambientLight *= vec3(GM_light_ambient);

    // 最终结果
    vec3 color = ambientLight + shadeFactor * (diffuseLight + specularLight);
    frag_color = vec4(color, 1.0f);
    //frag_color = vec4(1, 1, 1, 1.0f);
}

void main()
{
    drawObject();
}
