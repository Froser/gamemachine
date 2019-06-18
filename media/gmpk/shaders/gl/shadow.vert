void GM_Shadow()
{
    if (GM_UseAnimation == GM_SkeletalAnimation)
    {
        mat4 boneTransform = GM_Bones[boneIDs[0]] * weights[0];
        boneTransform += GM_Bones[boneIDs[1]] * weights[1];
        boneTransform += GM_Bones[boneIDs[2]] * weights[2];
        boneTransform += GM_Bones[boneIDs[3]] * weights[3];
        position = boneTransform * position;
    }
    else if (GM_UseAnimation == GM_AffineAnimation)
    {
        // 将仿射变换的值放到第1个Bones矩阵中
        position = GM_Bones[0] * position;
    }
    
    gl_Position = GM_ShadowInfo.ShadowMatrix[GM_ShadowInfo.CurrentCascadeLevel] * GM_WorldMatrix * position;
}