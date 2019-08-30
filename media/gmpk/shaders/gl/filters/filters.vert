#if GL_ES
precision mediump int;
precision mediump float;
precision mediump sampler2DShadow;
#endif

out vec2 _texCoords;

void main()
{
    init_layouts();
    vec2 uv_gl = vec2(uv.x, 1.f - uv.y);
    gl_Position = position;
    _texCoords = uv_gl;
    end_init_layouts();
}
