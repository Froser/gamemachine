#include "stdafx.h"
#include "gmgl_renders_lightpass.h"
#include "foundation/gamemachine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglshaderprogram.h"

GMGLRenders_LightPass::GMGLRenders_LightPass()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}
