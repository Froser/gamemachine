out vec4 _model3d_position_world;

void model3d_calcCoords()
{
    if (GM_UseAnimation == GM_SkeletalAnimation)
    {
        mat4 boneTransform = GM_Bones[boneIDs[0]] * weights[0];
        boneTransform += GM_Bones[boneIDs[1]] * weights[1];
        boneTransform += GM_Bones[boneIDs[2]] * weights[2];
        boneTransform += GM_Bones[boneIDs[3]] * weights[3];
        position = boneTransform * position;
        normal = vec4(mat3(boneTransform) * normal.xyz, 1);
    }
    else if (GM_UseAnimation == GM_AffineAnimation)
    {
        // 将仿射变换的值放到第1个Bones矩阵中
        position = GM_Bones[0] * position;
        normal = vec4(mat3(GM_Bones[0]) * normal.xyz, 1);
    }

    _model3d_position_world = GM_WorldMatrix * position;
    _position = position;
    _normal = normal;
    gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * _model3d_position_world;
}

void GM_Model3D()
{
    model3d_calcCoords();
}