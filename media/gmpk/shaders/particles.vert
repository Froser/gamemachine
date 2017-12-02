void particles_main(void)
{
    _uv = uv;
    _color = color;
    gl_Position = GM_projection_matrix * GM_view_matrix * position;
}