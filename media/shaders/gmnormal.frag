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
uniform float GM_ambient_texture_scroll_s;
uniform float GM_ambient_texture_scroll_t;
uniform float GM_ambient_texture_scale_s;
uniform float GM_ambient_texture_scale_t;
uniform int GM_ambient_texture_switch = 0;

uniform sampler2D GM_ambient_texture_2;
uniform float GM_ambient_texture_2_scroll_s;
uniform float GM_ambient_texture_2_scroll_t;
uniform float GM_ambient_texture_2_scale_s;
uniform float GM_ambient_texture_2_scale_t;
uniform int GM_ambient_texture_2_switch = 0;

uniform sampler2D GM_ambient_texture_3;
uniform float GM_ambient_texture_3_scroll_s;
uniform float GM_ambient_texture_3_scroll_t;
uniform float GM_ambient_texture_3_scale_s;
uniform float GM_ambient_texture_3_scale_t;
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
uniform vec3 GM_light_ambient;
uniform vec3 GM_light_ka;
uniform vec3 GM_light_specular;
uniform vec3 GM_light_kd;
uniform vec3 GM_light_ks;
uniform vec3 GM_light_ke;
uniform vec3 GM_light_position;
uniform float GM_light_shininess;
uniform mat4 GM_model_matrix;

// 调试变量
uniform int GM_debug_draw_normal;

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
    vec3 N = normalize(normal);
    vec3 L = normalize(lightDirection);

    //diffuse:
    {
        g_diffuse = dot(L, N);
        g_diffuse = clamp(g_diffuse, 0.0f, 1.0f);
    }

    // specular
    {
        vec3 V = normalize(eyeDirection);
        vec3 R = reflect(-L, N);
        float theta = dot(V, R);
        g_specular = pow(theta, GM_light_shininess);
        g_specular = clamp(g_specular, 0.0f, 1.0f);
    }
    g_specular = 0;
}

vec3 normal_eye;
void calcLights()
{
    vec4 vertex_eye = GM_view_matrix * position_world;
    vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;
    vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_light_position, 1)).xyz;
    vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;

    // 由顶点变换矩阵计算法向量变换矩阵
    mat4 normalModelTransform = transpose(inverse(GM_model_matrix));
    mat4 normalEyeTransform = GM_view_matrix * normalModelTransform;

    // normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
    normal_eye = normalize( (normalEyeTransform * vec4(_normal.xyz, 0)).xyz );

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

        vec3 lightDirection_tangent = TBN * lightDirection_eye;
        vec3 eyeDirection_tangent = TBN * eyeDirection_eye;
        vec3 normal_tangent = texture(GM_normal_mapping_texture, _uv).rgb * 2.0 - 1.0;

        calcDiffuseAndSpecular(lightDirection_tangent, eyeDirection_tangent, normal_tangent);
        //DEBUG.rgb = _tangent.xyz;
    }
}

void drawObject()
{
    calcLights();

    if (GM_debug_draw_normal == 1)
    {
        // 画眼睛视角的法向量
        frag_color = vec4((normal_eye.xyz + 1.f) / 2.f, 1.f);
        return;
    }
    else if (GM_debug_draw_normal == 2)
    {
        // 画世界视觉的法向量
        frag_color = vec4((_normal.xyz + 1.f) / 2.f, 1.f);
        return;
    }

    // 计算阴影系数
    float shadeFactor = shadeFactorFactor(calcuateShadeFactor(shadowCoord));

    // 环境光
    vec3 ambientLight = vec3(0);

    // 计算点光源（漫反射、Kd和镜面反射）
    vec3 diffuseTextureColor = GM_diffuse_texture_switch == 1 ? vec3(texture(GM_diffuse_texture, _uv)) : vec3(1);
    vec3 diffuseLight = g_diffuse * GM_light_kd * diffuseTextureColor;
    diffuseLight *= GM_light_specular;

    vec3 specularLight = g_specular * GM_light_specular * GM_light_ks;

    // 计算环境光和Ka贴图
    vec3 ambientTextureColor = GM_ambient_texture_switch == 1 ? vec3(texture(GM_ambient_texture, _uv * vec2(GM_ambient_texture_scale_s, GM_ambient_texture_scale_t) + vec2(GM_ambient_texture_scroll_s, GM_ambient_texture_scroll_t))) : vec3(1);
    ambientTextureColor += GM_ambient_texture_2_switch == 1 ? vec3(texture(GM_ambient_texture_2, _uv * vec2(GM_ambient_texture_2_scale_s, GM_ambient_texture_2_scale_t) + vec2(GM_ambient_texture_2_scroll_s, GM_ambient_texture_2_scroll_t))) : vec3(0);
    ambientTextureColor += GM_ambient_texture_3_switch == 1 ? vec3(texture(GM_ambient_texture_3, _uv * vec2(GM_ambient_texture_3_scale_s, GM_ambient_texture_3_scale_t) + vec2(GM_ambient_texture_3_scroll_s, GM_ambient_texture_3_scroll_t))) : vec3(0);
    ambientTextureColor *= GM_lightmap_texture_switch == 1 ? vec3(texture(GM_lightmap_texture, _lightmapuv)) : vec3(1);
    ambientLight += GM_light_ka * shadeFactor * ambientTextureColor;
    ambientLight *= GM_light_ambient;

    // 最终结果
    vec3 color = ambientLight + shadeFactor * (diffuseLight + specularLight);
    frag_color = vec4(color, 1.0f);
}

void main()
{
    drawObject();
}
