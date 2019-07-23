void GM_Model2D()
{
    gl_Position = GM_WorldMatrix * position;
    _position = position;
    _uv = uv;
    _color = color;
}