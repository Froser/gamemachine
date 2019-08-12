#version @@@GMGL_SHADER_VERSION@@@

#include "foundation/foundation.h"
#include "foundation/properties.h"
#include "foundation/frag_header.h"
#include "foundation/light.h"

// FRAGMENT
#include "model2d.frag"
#include "model3d.frag"
#include "text.frag"
#include "cubemap.frag"
#include "shadow.frag"
#include "particle.frag"

#include "foundation/invoke.h"

void main(void)
{
    GM_InvokeTechniqueEntrance();
}