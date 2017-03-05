#include "stdafx.h"
#include "bspphysicsworldprivate.h"

GMint BSP_Physics_Plane::classifyBox(vmath::vec3& emins, vmath::vec3& emaxs)
{
	GMfloat dist1, dist2;
	GMint sides;

	// fast axial cases
	if (planeType < PLANE_NON_AXIAL)
	{
		if (plane->intercept <= emins[planeType])
			return 1;
		if (plane->intercept >= emaxs[planeType])
			return 2;
		return 3;
	}

	// general case
	switch (signbits)
	{
	case 0:
		dist1 = plane->normal[0] * emaxs[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emaxs[2];
		dist2 = plane->normal[0] * emins[0] + plane->normal[1] * emins[1] + plane->normal[2] * emins[2];
		break;
	case 1:
		dist1 = plane->normal[0] * emins[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emaxs[2];
		dist2 = plane->normal[0] * emaxs[0] + plane->normal[1] * emins[1] + plane->normal[2] * emins[2];
		break;
	case 2:
		dist1 = plane->normal[0] * emaxs[0] + plane->normal[1] * emins[1] + plane->normal[2] * emaxs[2];
		dist2 = plane->normal[0] * emins[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emins[2];
		break;
	case 3:
		dist1 = plane->normal[0] * emins[0] + plane->normal[1] * emins[1] + plane->normal[2] * emaxs[2];
		dist2 = plane->normal[0] * emaxs[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emins[2];
		break;
	case 4:
		dist1 = plane->normal[0] * emaxs[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emins[2];
		dist2 = plane->normal[0] * emins[0] + plane->normal[1] * emins[1] + plane->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = plane->normal[0] * emins[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emins[2];
		dist2 = plane->normal[0] * emaxs[0] + plane->normal[1] * emins[1] + plane->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = plane->normal[0] * emaxs[0] + plane->normal[1] * emins[1] + plane->normal[2] * emins[2];
		dist2 = plane->normal[0] * emins[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = plane->normal[0] * emins[0] + plane->normal[1] * emins[1] + plane->normal[2] * emins[2];
		dist2 = plane->normal[0] * emaxs[0] + plane->normal[1] * emaxs[1] + plane->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		break;
	}

	sides = 0;
	if (dist1 >= plane->intercept)
		sides = 1;
	if (dist2 < plane->intercept)
		sides |= 2;

	return sides;
}