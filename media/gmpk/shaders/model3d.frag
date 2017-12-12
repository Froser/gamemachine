// 相机视角法向量
vec3 g_model3d_normal_eye;
float g_model3d_shadeFactor = 0;
vec3 g_model3d_ambientLight;
vec3 g_model3d_diffuseLight;
vec3 g_model3d_specularLight;
in vec4 _model3d_position_world;

void model3d_init()
{
	g_model3d_ambientLight = g_model3d_diffuseLight = g_model3d_specularLight = vec3(0);
}

float model3d_calcuateShadeFactor(vec4 shadowCoord)
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

float model3d_shadeFactorFactor(float shadeFactor)
{
	return min(shadeFactor + 0.3, 1);
}

void model3d_calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		g_model3d_diffuseLight += diffuseFactor * GM_material.kd * g_model3d_shadeFactor * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, GM_material.shininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		g_model3d_specularLight += specularFactor * GM_material.ks * g_model3d_shadeFactor * light.lightColor;
	}
}

void model3d_calcLights()
{
	//g_model3d_shadeFactor = model3d_shadeFactorFactor(model3d_calcuateShadeFactor(_shadowCoord));
	g_model3d_shadeFactor = 1;

	// 由顶点变换矩阵计算法向量变换矩阵
	mat4 noTranslationMat = gm_removeTranslation(GM_model_matrix);
	mat4 normalEyeTransform = GM_view_matrix * GM_inverse_transpose_model_matrix;
	vec4 vertex_eye = GM_view_matrix * _model3d_position_world;
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	g_model3d_normal_eye = normalize( (normalEyeTransform * vec4(_normal.xyz, 0)).xyz );

	// 计算漫反射和高光部分
	if (GM_normalmap_textures[0].enabled == 0)
	{
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			model3d_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_eye, eyeDirection_eye, g_model3d_normal_eye);
		}
	}
	else
	{
		vec3 normal_tangent = texture(GM_normalmap_textures[0].texture, _uv).rgb * 2.0 - 1.0;
		vec3 tangent_eye = normalize((normalEyeTransform * vec4(_tangent.xyz, 0)).xyz);
		vec3 bitangent_eye = normalize((normalEyeTransform * vec4(_bitangent.xyz, 0)).xyz);
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			mat3 TBN = transpose(mat3(
				tangent_eye,
				bitangent_eye,
				g_model3d_normal_eye.xyz
			));
			vec3 lightDirection_tangent = TBN * lightDirection_eye;
			vec3 eyeDirection_tangent = TBN * eyeDirection_eye;

			model3d_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
	}

	// 计算环境光
	for (int i = 0; i < GM_ambients_count; i++)
	{
		g_model3d_ambientLight += GM_material.ka * GM_ambients[i].lightColor * g_model3d_shadeFactor;
	}
}

vec3 model3d_calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
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

void model3d_calcColor()
{
	model3d_calcLights();

	if (GM_debug_draw_normal == 1)
	{
		// 画眼睛视角的法向量
		_frag_color = vec4((g_model3d_normal_eye.xyz + 1.f) / 2.f, 1.f);
		return;
	}
	else if (GM_debug_draw_normal == 2)
	{
		// 画世界视角的法向量
		_frag_color = vec4((_normal.xyz + 1.f) / 2.f, 1.f);
		return;
	}
	vec3 diffuseTextureColor = model3d_calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	vec3 ambientTextureColor = model3d_calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) *
		model3d_calcTexture(GM_lightmap_textures, _lightmapuv, 1);

	// 最终结果
	vec3 color = g_model3d_ambientLight * ambientTextureColor + g_model3d_diffuseLight * diffuseTextureColor + g_model3d_specularLight;
	_frag_color = vec4(color, 1.0f);
}

void model3d_main()
{
	model3d_init();
	model3d_calcColor();
}
