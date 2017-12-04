#alias deferred_material_pass_gKa			deferred_geometry_pass_slot_0
#alias deferred_material_pass_gKd			deferred_geometry_pass_slot_1
#alias deferred_material_pass_gKs			deferred_geometry_pass_slot_2
#alias deferred_material_pass_gShininess	deferred_geometry_pass_slot_3
#alias deferred_material_pass_gHasNormalMap	deferred_geometry_pass_slot_4

void deferred_material_pass_main()
{
	${deferred_material_pass_gKa} = GM_material.ka;
	${deferred_material_pass_gKd} = GM_material.kd;
	${deferred_material_pass_gKs} = GM_material.ks;
	${deferred_material_pass_gShininess} = vec3(GM_material.shininess, 0, 0);
	${deferred_material_pass_gHasNormalMap} = GM_normalmap_textures[0].enabled == 0 ? vec3(-1, 0, 0) : vec3(1, 0, 0);
}
