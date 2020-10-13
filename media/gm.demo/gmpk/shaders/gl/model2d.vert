void GM_Model2D()
{
    gl_Position = GM_WorldMatrix * position;
    _uv = uv;
    _color = color;
}