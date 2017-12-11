in vec4 _deferred_geometry_pass_position_world;

#alias deferred_geometry_pass_gPosition			deferred_geometry_pass_slot_0
#alias deferred_geometry_pass_gNormal_eye		deferred_geometry_pass_slot_1
#alias deferred_geometry_pass_gTexAmbient		deferred_geometry_pass_slot_2
#alias deferred_geometry_pass_gTexDiffuse		deferred_geometry_pass_slot_3
#alias deferred_geometry_pass_gTangent_eye		deferred_geometry_pass_slot_4
#alias deferred_geometry_pass_gBitangent_eye	deferred_geometry_pass_slot_5
#alias deferred_geometry_pass_gNormalMap		deferred_geometry_pass_slot_6

vec3 deferred_geometry_pass_calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
{
	bool hasTexture = false;
	vec3 result = vec3(0);
	for (int i = 0; i < size; i++)
	{
		if (textures[i].enabled == 0)
			break;
		
		result += textures[i].enabled == 1
			? vec3(texture(textures[i].texture, uv * vec2(textures[i].scale_s, textures[i].scale_t) + vec2(textures[i].scroll_s, textures[i].scroll_t)))
			: vec3(0);
		if (textures[i].enabled == 1)
			hasTexture = true;
	}

	if (!hasTexture)
		return vec3(1);

	return result;
}

void deferred_geometry_pass_calcEyeSpace()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	mat4 noTranslationMat = gm_removeTranslation(GM_model_matrix);
	mat4 normalModelTransform = transpose(inverse(noTranslationMat)); //TODO 这个计算很耗时，应该用uniform传入
	mat4 normalEyeTransform = GM_view_matrix * normalModelTransform;
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	${deferred_geometry_pass_gNormal_eye} = normalToTexture ( normalize( (normalEyeTransform * vec4(_normal.xyz, 0)).xyz ) );

	if (GM_normalmap_textures[0].enabled == 1)
	{
		${deferred_geometry_pass_gTangent_eye} = normalize((normalEyeTransform * vec4(_tangent.xyz, 0)).xyz);
		${deferred_geometry_pass_gBitangent_eye} = normalize((normalEyeTransform * vec4(_bitangent.xyz, 0)).xyz);
	}
	else
	{
		${deferred_geometry_pass_gTangent_eye} = vec3(0,0,0);
		${deferred_geometry_pass_gBitangent_eye} = vec3(0,0,0);
	}
}

void deferred_geometry_pass_main()
{
	${deferred_geometry_pass_gPosition} = _deferred_geometry_pass_position_world.xyz;
	${deferred_geometry_pass_gTexAmbient} = deferred_geometry_pass_calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) * deferred_geometry_pass_calcTexture(GM_lightmap_textures, _lightmapuv, 1);
	${deferred_geometry_pass_gTexDiffuse} = deferred_geometry_pass_calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	${deferred_geometry_pass_gNormalMap} = texture(GM_normalmap_textures[0].texture, _uv).rgb;
	deferred_geometry_pass_calcEyeSpace();
}
