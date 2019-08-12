#version @@@GMGL_SHADER_VERSION@@@

#unused GM_GammaCorrection
#unused GM_Gamma
#unused GM_GammaInv

#include "foundation/foundation.h"
#include "foundation/vert_header.h"

// VERTEX
#include "model2d.vert"
#include "model3d.vert"
#include "text.vert"
#include "cubemap.vert"
#include "shadow.vert"
#include "particle.vert"

#include "foundation/invoke.h"

void main(void)
{
    init_layouts();

    GM_InvokeTechniqueEntrance();

    end_init_layouts();
}
