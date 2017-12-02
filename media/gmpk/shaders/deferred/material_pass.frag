layout (location = 0) out vec3 deferred_material_pass_gKa;
layout (location = 1) out vec3 deferred_material_pass_gKd;
layout (location = 2) out vec3 deferred_material_pass_gKs;
layout (location = 3) out vec3 deferred_material_pass_gShininess;
layout (location = 4) out vec3 deferred_material_pass_gHasNormalMap;

void deferred_material_pass_main()
{
	deferred_material_pass_gKa = GM_material.ka;
	deferred_material_pass_gKd = GM_material.kd;
	deferred_material_pass_gKs = GM_material.ks;
	deferred_material_pass_gShininess = vec3(GM_material.shininess, 0, 0);
	deferred_material_pass_gHasNormalMap = GM_normalmap_textures[0].enabled == 0 ? vec3(-1, 0, 0) : vec3(1, 0, 0);
}
