#version 330 core

// 环境立方体纹理（绘制天空时）
uniform sampler2D GM_ambient_texture;
uniform int GM_ambient_texture_switch = 0;
uniform vec4 GM_light_ambient;

in vec2 _uv;
out vec4 frag_color;

void drawSky()
{
    vec3 cubemapTextureColor = vec3(texture(GM_ambient_texture, _uv));
    frag_color = GM_light_ambient * vec4(cubemapTextureColor, 1.0f);
    //frag_color = vec4(_uv.x, _uv.y, 0, 1);
}

void main()
{
    drawSky();
}
