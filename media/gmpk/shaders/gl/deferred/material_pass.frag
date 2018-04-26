#alias deferred_material_pass_gKs											deferred_geometry_pass_slot_0
#alias deferred_material_pass_gShininess_gHasNormalMap_gRefractivity		deferred_geometry_pass_slot_1

subroutine (GM_TechniqueEntrance) void GM_MaterialPass()
{
	${deferred_material_pass_gKs} = vec4(GM_material.ks, 0);
	
	${deferred_material_pass_gShininess_gHasNormalMap_gRefractivity} = vec4(
		GM_material.shininess,
		GM_normalmap_textures[0].enabled == 0 ? 0 : 1,
		GM_material.refractivity,
		0);
}
