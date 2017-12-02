uniform sampler2D deferred_light_pass_gPosition;
uniform sampler2D deferred_light_pass_gNormal_eye;
uniform sampler2D deferred_light_pass_gTexAmbient;
uniform sampler2D deferred_light_pass_gTexDiffuse;
uniform sampler2D deferred_light_pass_gTangent_eye;
uniform sampler2D deferred_light_pass_gBitangent_eye;
uniform sampler2D deferred_light_pass_gNormalMap;
uniform sampler2D deferred_light_pass_gKa;
uniform sampler2D deferred_light_pass_gKd;
uniform sampler2D deferred_light_pass_gKs;
uniform sampler2D deferred_light_pass_gShininess;
uniform sampler2D deferred_light_pass_gHasNormalMap;

// geometries
vec3 deferred_light_pass_tPosition;
vec3 deferred_light_pass_tNormal_eye;
vec3 deferred_light_pass_tTexAmbient;
vec3 deferred_light_pass_tTexDiffuse;
vec3 deferred_light_pass_tTangent_eye;
vec3 deferred_light_pass_tBitangent_eye;
vec3 deferred_light_pass_tNormalMap;

// materials
vec3 deferred_light_pass_tKa;
vec3 deferred_light_pass_tKd;
vec3 deferred_light_pass_tKs;
float deferred_light_pass_tShininess;

// flags, -1 or 1
float deferred_light_pass_tHasNormalMap;

// 相机视角法向量
vec3 deferred_light_pass_g_normal_eye;
// ShadowMap的阴影系数，如果没有ShadowMap则为1
vec3 deferred_light_pass_g_ambientLight;
vec3 deferred_light_pass_g_diffuseLight;
vec3 deferred_light_pass_g_specularLight;

bool hasFlag(float flag)
{
	return flag > 0;
}

void deferred_light_pass_init()
{
	deferred_light_pass_g_ambientLight = deferred_light_pass_g_diffuseLight = deferred_light_pass_g_specularLight = vec3(0);
	deferred_light_pass_tPosition = texture(deferred_light_pass_gPosition, _uv).rgb;
	deferred_light_pass_tNormal_eye = texture(deferred_light_pass_gNormal_eye, _uv).rgb;
	deferred_light_pass_tTexAmbient = texture(deferred_light_pass_gTexAmbient, _uv).rgb;
	deferred_light_pass_tTexDiffuse = texture(deferred_light_pass_gTexDiffuse, _uv).rgb;
	deferred_light_pass_tTangent_eye = texture(deferred_light_pass_gTangent_eye, _uv).rgb;
	deferred_light_pass_tBitangent_eye = texture(deferred_light_pass_gBitangent_eye, _uv).rgb;
	deferred_light_pass_tNormalMap = texture(deferred_light_pass_gNormalMap, _uv).rgb;
	deferred_light_pass_tKa = texture(deferred_light_pass_gKa, _uv).rgb;
	deferred_light_pass_tKd = texture(deferred_light_pass_gKd, _uv).rgb;
	deferred_light_pass_tKs = texture(deferred_light_pass_gKs, _uv).rgb;
	deferred_light_pass_tShininess = texture(deferred_light_pass_gShininess, _uv).r;
	deferred_light_pass_tHasNormalMap = texture(deferred_light_pass_gHasNormalMap, _uv).r;
}

void deferred_light_pass_calcDiffuseAndSpecular(GM_light_t light, vec3 lightDirection, vec3 eyeDirection, vec3 normal)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDirection);

	//diffuse:
	{
		float diffuseFactor = dot(L, N);
		diffuseFactor = clamp(diffuseFactor, 0.0f, 1.0f);

		deferred_light_pass_g_diffuseLight += diffuseFactor * deferred_light_pass_tKd * light.lightColor;
	}

	// specular:
	{
		vec3 V = normalize(eyeDirection);
		vec3 R = reflect(-L, N);
		float theta = dot(V, R);
		float specularFactor = pow(theta, deferred_light_pass_tShininess);
		specularFactor = clamp(specularFactor, 0.0f, 1.0f);

		deferred_light_pass_g_specularLight += specularFactor * deferred_light_pass_tKs * light.lightColor;
	}
}

void deferred_light_pass_calcLights()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	vec4 vertex_eye = GM_view_matrix * vec4(deferred_light_pass_tPosition, 1);
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;

	// 计算漫反射和高光部分
	if (!hasFlag(deferred_light_pass_tHasNormalMap))
	{
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			deferred_light_pass_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_eye, eyeDirection_eye, deferred_light_pass_tNormal_eye);
		}
	}
	else
	{
		vec3 normal_tangent = deferred_light_pass_tNormalMap.rgb * 2.0 - 1.0;
		for (int i = 0; i < GM_speculars_count; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_speculars[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			mat3 TBN = transpose(mat3(
				deferred_light_pass_tTangent_eye,
				deferred_light_pass_tBitangent_eye,
				deferred_light_pass_tNormal_eye
			));
			vec3 lightDirection_tangent = TBN * lightDirection_eye;
			vec3 eyeDirection_tangent = TBN * eyeDirection_eye;

			deferred_light_pass_calcDiffuseAndSpecular(GM_speculars[i], lightDirection_tangent, eyeDirection_tangent, normal_tangent);
		}
	}

	// 计算环境光
	for (int i = 0; i < GM_ambients_count; i++)
	{
		deferred_light_pass_g_ambientLight += deferred_light_pass_tKa * GM_ambients[i].lightColor;
	}
}

void deferred_light_pass_calcColor()
{
	deferred_light_pass_calcLights();

	// 最终结果
	vec3 color = deferred_light_pass_g_ambientLight * deferred_light_pass_tTexAmbient + deferred_light_pass_g_diffuseLight * deferred_light_pass_tTexDiffuse + deferred_light_pass_g_specularLight;
	_frag_color = vec4(color, 1.0f);
}

void deferred_light_pass_main()
{
	deferred_light_pass_init();
	deferred_light_pass_calcColor();
}
