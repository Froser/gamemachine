void GM_Text(void)
{
    gl_Position = GM_WorldMatrix * position;
    gl_Position.z = 0.f;
    gl_Position.w = 1.f;
    _position = position;
    _uv = uv;
    _color = color;
}