out vec4 _model3d_position_world;

void model3d_calcCoords()
{
	if (GM_UseBoneAnimation != 0)
	{
		mat4 boneTransform = GM_Bones[boneIDs[0]] * weights[0];
		boneTransform += GM_Bones[boneIDs[1]] * weights[1];
		boneTransform += GM_Bones[boneIDs[2]] * weights[2];
		boneTransform += GM_Bones[boneIDs[3]] * weights[3];
		position = boneTransform * position;
		normal = boneTransform * vec4(normal.xyz, 0);
	}

    _model3d_position_world = GM_WorldMatrix * position;
    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * _model3d_position_world;
    _normal = normal;
    _tangent = tangent;
    _bitangent = bitangent;
    _uv = uv;
    _lightmapuv = lightmapuv;
    _color = color;
}

void GM_Model3D()
{
    model3d_calcCoords();
}