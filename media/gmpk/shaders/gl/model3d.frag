// 相机视角法向量
vec3 g_model3d_normal_eye;
vec3 g_model3d_ambientLight;
vec3 g_model3d_diffuseLight;
vec3 g_model3d_specularLight;
vec3 g_model3d_refractionLight;

in vec4 _model3d_position_world;

vec3 model3d_calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
{
	vec3 result = vec3(0);
	for (int i = 0; i < size; i++)
	{
		if (textures[i].enabled == 0)
			break;
		
		result += textures[i].enabled == 1
			? vec3(texture(textures[i].texture, uv * vec2(textures[i].scale_s, textures[i].scale_t) + vec2(textures[i].scroll_s, textures[i].scroll_t)))
			: vec3(0);
	}
	return result;
}

subroutine (GM_TechniqueEntrance)
void GM_Model3D()
{
	PS_3D_INPUT vertex;
	vertex.WorldPos = _model3d_position_world.xyz;

	mat3 inverse_transpose_model_matrix = mat3(GM_inverse_transpose_model_matrix);
	vertex.Normal_World_N = normalize(inverse_transpose_model_matrix * _normal.xyz);

	mat3 normalEyeTransform = mat3(GM_view_matrix) * inverse_transpose_model_matrix;
	vertex.Normal_Eye_N = normalize( (normalEyeTransform * _normal.xyz) );

	TangentSpace tangentSpace;
	vec3 tangent_eye = normalize(normalEyeTransform * _tangent.xyz);
	vec3 bitangent_eye = normalize(normalEyeTransform * _bitangent.xyz);
	mat3 TBN = transpose(mat3(
		tangent_eye,
		bitangent_eye,
		vertex.Normal_Eye_N
	));
	tangentSpace.TBN = TBN;
	tangentSpace.Normal_Tangent_N = texture(GM_normalmap_textures[0].texture, _uv).rgb * 2.0 - 1.0;
	vertex.TangentSpace = tangentSpace;

	vertex.HasNormalMap = GM_normalmap_textures[0].enabled != 0;
	vertex.AmbientLightmapTexture = model3d_calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT)
		 * model3d_calcTexture(GM_lightmap_textures, _lightmapuv, 1)
		 * GM_material.ka;
	vertex.DiffuseTexture = model3d_calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT) * GM_material.kd;

	vertex.SpecularTexture = model3d_calcTexture(GM_specular_textures, _uv, MAX_TEXTURE_COUNT).r * GM_material.ks;
	vertex.Shininess = GM_material.shininess;
	vertex.Refractivity = GM_material.refractivity;

	_frag_color = PS_3D_CalculateColor(vertex);
}
