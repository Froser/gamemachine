#version 330 core

// 环境立方体纹理（绘制天空时）
uniform samplerCube GM_cubemap_texture;
uniform vec4 GM_light_ambient;

in vec3 _uv;
out vec4 frag_color;

void drawSky()
{
    vec3 cubemapTextureColor = vec3(texture(GM_cubemap_texture, _uv));
    frag_color = GM_light_ambient * vec4(cubemapTextureColor, 1.0f);
}

void main()
{
    drawSky();
}
