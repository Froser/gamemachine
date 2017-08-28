#include "../foundation/vert_header.h"

void main(void)
{
    gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
}