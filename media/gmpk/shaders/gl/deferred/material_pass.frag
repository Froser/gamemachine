#alias deferred_material_pass_gKa								deferred_geometry_pass_slot_0
#alias deferred_material_pass_gKd								deferred_geometry_pass_slot_1
#alias deferred_material_pass_gKs_gShininess					deferred_geometry_pass_slot_2
#alias deferred_material_pass_gHasNormalMap_gRefractivity		deferred_geometry_pass_slot_3

subroutine (GM_TechniqueEntrance) void GM_MaterialPass()
{
	${deferred_material_pass_gKa} = vec4(GM_material.ka, 0);
	${deferred_material_pass_gKd} = vec4(GM_material.kd, 0);
	${deferred_material_pass_gKs_gShininess} = vec4(GM_material.ks, GM_material.shininess);
	
	${deferred_material_pass_gHasNormalMap_gRefractivity} = vec4(
		GM_normalmap_textures[0].enabled == 0 ? -1 : 1,
		GM_material.refractivity,
		0,
		0);
}
