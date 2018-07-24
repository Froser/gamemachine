#version 410
#include "foundation/foundation.h"
#include "foundation/vert_header.h"

// VERTEX
#include "model2d.vert"
#include "model3d.vert"
#include "text.vert"
#include "cubemap.vert"
#include "shadow.vert"
#include "particle.vert"

void main(void)
{
    init_layouts();
    GM_techniqueEntrance();
}