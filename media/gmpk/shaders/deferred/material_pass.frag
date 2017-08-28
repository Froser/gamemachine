#version 330 core
#include "../foundation/properties.h"

layout (location = 0) out vec3 gKa;
layout (location = 1) out vec3 gKd;
layout (location = 2) out vec3 gKs;
layout (location = 3) out vec3 gShininess;
layout (location = 4) out vec3 gHasNormalMap;

void main()
{
	gKa = GM_material.ka;
	gKd = GM_material.kd;
	gKs = GM_material.ks;
	gShininess = vec3(GM_material.shininess, 0, 0);
	gHasNormalMap = GM_normalmap_textures[0].enabled == 0 ? vec3(-1, 0, 0) : vec3(1, 0, 0);
}
