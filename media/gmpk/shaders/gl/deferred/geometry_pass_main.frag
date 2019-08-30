layout (location = 0) out vec4 deferred_geometry_pass_slot_0;
layout (location = 1) out vec4 deferred_geometry_pass_slot_1;
layout (location = 2) out vec4 deferred_geometry_pass_slot_2;
layout (location = 3) out vec4 deferred_geometry_pass_slot_3;
layout (location = 4) out vec4 deferred_geometry_pass_slot_4;
layout (location = 5) out vec4 deferred_geometry_pass_slot_5;
layout (location = 6) out vec4 deferred_geometry_pass_slot_6;
layout (location = 7) out vec4 deferred_geometry_pass_slot_7;

vec4 normalToTexture(vec3 normal)
{
    return vec4((normal + 1.f) * .5f, 1.f);
}
in vec4 _deferred_geometry_pass_position_world;

void GM_GeometryPass()
{
    deferred_geometry_pass_gPosition_Refractivity.rgb = _deferred_geometry_pass_position_world.rgb;
    deferred_geometry_pass_gPosition_Refractivity.a = GM_Material.Refractivity;

    if (GM_IlluminationModel == GM_IlluminationModel_None)
    {
        discard;
    }
    else if (GM_IlluminationModel == GM_IlluminationModel_Phong)
    {
        deferred_geometry_pass_gTexAmbientAlbedo = vec4(GM_Material.Ka, 1) * GM_SampleTextures(GM_AmbientTextureAttribute, _uv) * GM_SampleTextures(GM_LightmapTextureAttribute, _lightmapuv);
        deferred_geometry_pass_gTexDiffuseMetallicRoughnessAO = vec4(GM_Material.Kd, 1) * GM_SampleTextures(GM_DiffuseTextureAttribute, _uv);
        deferred_geometry_pass_gKs_Shininess_F0 = vec4(GM_Material.Ks * GM_SampleTextures(GM_SpecularTextureAttribute, _uv).r, GM_Material.Shininess);
    }
    else if (GM_IlluminationModel == GM_IlluminationModel_CookTorranceBRDF)
    {
        deferred_geometry_pass_gTexAmbientAlbedo = GM_SampleTextures(GM_AlbedoTextureAttribute, _uv);
        deferred_geometry_pass_gTexDiffuseMetallicRoughnessAO = GM_SampleTextures(GM_MetallicRoughnessAOTextureAttribute, _uv);
        deferred_geometry_pass_gKs_Shininess_F0 = vec4(GM_Material.F0, 1);
    }

    // 由顶点变换矩阵计算法向量变换矩阵
    // normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
    vec3 normal_World_N = normalize( mat3(GM_InverseTransposeModelMatrix) * _normal.xyz);
    deferred_geometry_pass_gNormal_IlluminationModel = normalToTexture ( normal_World_N );
    deferred_geometry_pass_gNormal_IlluminationModel.a = float(GM_IlluminationModel);

    if (GM_NormalMapTextureAttribute.Enabled == 1)
    {
        deferred_geometry_pass_gNormalMap_bNormalMap = GM_SampleTextures(GM_NormalMapTextureAttribute, _uv);
        deferred_geometry_pass_gNormalMap_bNormalMap.a = 1.f;
        if (GM_IsTangentSpaceInvalid(_tangent.xyz, _bitangent.xyz))
        {
            GMTangentSpace tangentSpace = GM_CalculateTangentSpaceRuntime(
                _deferred_geometry_pass_position_world.xyz,
                _uv,
                normal_World_N,
                GM_NormalMapTextureAttribute
            );
            mat3 TBN = transpose(tangentSpace.TBN);
            deferred_geometry_pass_gTangent_eye = normalToTexture(normalize(TBN[0]));
            deferred_geometry_pass_gBitangent_eye = normalToTexture(normalize(TBN[1]));
        }
        else
        {
            mat3 normalEyeTransform = mat3(GM_ViewMatrix * GM_InverseTransposeModelMatrix);
            deferred_geometry_pass_gTangent_eye = normalToTexture(normalize(normalEyeTransform * _tangent.xyz).xyz);
            deferred_geometry_pass_gBitangent_eye = normalToTexture(normalize(normalEyeTransform * _bitangent.xyz).xyz);
        }
    }
    else
    {
        deferred_geometry_pass_gTangent_eye = normalToTexture(vec3(0, 0, 0));
        deferred_geometry_pass_gBitangent_eye = normalToTexture(vec3(0, 0, 0));
        deferred_geometry_pass_gNormalMap_bNormalMap = vec4(0, 0, 0, 0);
    }
}

void main(void)
{
    GM_GeometryPass();
}
