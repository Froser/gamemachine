// 相机视角法向量
vec3 g_model3d_normal_eye;
vec3 g_model3d_ambientLight;
vec3 g_model3d_diffuseLight;
vec3 g_model3d_specularLight;
vec3 g_model3d_refractionLight;

in vec4 _model3d_position_world;

subroutine (GM_TechniqueEntrance)
void GM_Model3D()
{
    PS_3D_INPUT vertex;
    vertex.WorldPos = _model3d_position_world.xyz;

    mat3 inverse_transpose_model_matrix = mat3(GM_inverse_transpose_model_matrix);
    vertex.Normal_World_N = normalize(inverse_transpose_model_matrix * _normal.xyz);

    mat3 normalEyeTransform = mat3(GM_view_matrix) * inverse_transpose_model_matrix;
    vec3 normal_World_N = normalEyeTransform * _normal.xyz;
    vertex.Normal_Eye_N = normalize( normal_World_N );

    GMTangentSpace tangentSpace;
    if (GM_IsTangentSpaceInvalid(_tangent.xyz, _bitangent.xyz))
    {
        tangentSpace = GM_CalculateTangentSpaceRuntime(vertex.WorldPos, _uv, normal_World_N, GM_normalmap_texture.texture);
    }
    else
    {
        vec3 tangent_eye = normalize(normalEyeTransform * _tangent.xyz);
        vec3 bitangent_eye = normalize(normalEyeTransform * _bitangent.xyz);
        mat3 TBN = transpose(mat3(
            tangent_eye,
            bitangent_eye,
            vertex.Normal_Eye_N
        ));
        tangentSpace.TBN = TBN;
        tangentSpace.Normal_Tangent_N = texture(GM_normalmap_texture.texture, _uv).rgb * 2.0 - 1.0;
    }

    vertex.TangentSpace = tangentSpace;

    vertex.HasNormalMap = GM_normalmap_texture.enabled != 0;
    vertex.IlluminationModel = GM_IlluminationModel;
    if (GM_IlluminationModel == GM_IlluminationModel_Phong)
    {
        vertex.Shininess = GM_material.shininess;
        vertex.Refractivity = GM_material.refractivity;
        vertex.AmbientLightmapTexture = sampleTextures(GM_ambient_texture, _uv).rgb
             * sampleTextures(GM_lightmap_texture, _lightmapuv).rgb
             * GM_material.ka;
        vertex.DiffuseTexture = sampleTextures(GM_diffuse_texture, _uv).rgb * GM_material.kd;
        vertex.SpecularTexture = sampleTextures(GM_specular_texture, _uv).r * GM_material.ks;
    }
    else if (GM_IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
    {
        vertex.AlbedoTexture = pow(sampleTextures(GM_albedo_texture, _uv).rgb, vec3(GM_Gamma));
        vertex.MetallicRoughnessAOTexture = sampleTextures(GM_metallic_roughness_ao_texture, _uv).rgb;
        vertex.F0 = GM_material.f0;
    }

    _frag_color = PS_3D_CalculateColor(vertex);
}
