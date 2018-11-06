#include "stdafx.h"
#include <gmskeleton.h>

void GMSkeletalVertexBoneData::reset()
{
	D(d);
	GM_ZeroMemory(d->ids);
	GM_ZeroMemory(d->weights);
}

void GMSkeletalVertexBoneData::addData(GMsize_t boneId, GMfloat weight)
{
	D(d);
	for (GMsize_t i = 0; i < GM_array_size(d->ids); ++i)
	{
		if (d->weights[i] == 0.f)
		{
			d->ids[i] = boneId;
			d->weights[i] = weight;
			return;
		}
	}

	// 如果超过了BonesPerVertex骨骼，才会走到这里
	GM_ASSERT(false);
}