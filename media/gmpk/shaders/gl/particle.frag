in vec4 _particle_position_world;

subroutine (GM_TechniqueEntrance)
void GM_Particle()
{
    /// Start Debug Option
    if (GM_Debug_Normal == GM_Debug_Normal_WorldSpace)
    {
        mat3 inverse_transpose_model_matrix = mat3(GM_InverseTransposeModelMatrix);
        vec3 normal_World_N = normalize(inverse_transpose_model_matrix * _normal.xyz);
        _frag_color = normalToTexture(normal_World_N);
        return;
    }
    else if (GM_Debug_Normal == GM_Debug_Normal_EyeSpace)
    {
        mat3 inverse_transpose_model_matrix = mat3(GM_InverseTransposeModelMatrix);
        mat3 normalEyeTransform = mat3(GM_ViewMatrix) * inverse_transpose_model_matrix;
        vec3 normal_Eye_N = normalize(normalEyeTransform * _normal.xyz);
        _frag_color = normalToTexture(normal_Eye_N);
        return;
    }
    /// End Debug Option

    if (GM_AmbientTextureAttribute.Enabled == 1)
    {
        _frag_color = texture(GM_AmbientTextureAttribute.Texture, _uv * vec2(GM_AmbientTextureAttribute.ScaleX, GM_AmbientTextureAttribute.ScaleY));
        _frag_color *= _color;
    }
    else
    {
        _frag_color = vec4(0, 0, 0, 0);
    }
}
