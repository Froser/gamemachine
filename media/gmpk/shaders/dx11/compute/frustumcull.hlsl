// Compute Shader For Frustum Cull
struct AABB_t
{
    float4 vertices[8];
};

struct result_t
{
    int visible;
};

cbuffer FrustumPlanes : register(b0)
{
    float4 frustumPlanes[6]; //Normal & intercept. X.N + intercept = 0
};

static const float EPSILON = 0.01f;
static const int POINT_ON_PLANE = 0;
static const int POINT_IN_FRONT_OF_PLANE = 1;
static const int POINT_BEHIND_PLANE = 2;

int classifyPoint(float3 pt, float4 plane)
{
    float distance = dot(pt, plane.xyz) + plane.w;
    if (distance > EPSILON)
        return POINT_IN_FRONT_OF_PLANE;

    if (distance < -EPSILON)
        return POINT_BEHIND_PLANE;

    return POINT_ON_PLANE;
}

bool isBoundingBoxInside(float4 planes[6], AABB_t aabb)
{
    for (int i = 0; i < 6; ++i)
    {
        //if a point is not behind this plane, try next plane
        if (classifyPoint(aabb.vertices[0].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[1].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[2].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[3].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[4].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[5].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[6].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(aabb.vertices[7].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;

        //All vertices of the box are behind this plane
        return false;
    }

    return true;
}

StructuredBuffer<AABB_t> BufferAABB : register(t0);
RWStructuredBuffer<result_t> BufferOut : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    BufferOut[DTid.x].visible = isBoundingBoxInside(frustumPlanes, BufferAABB[DTid.x]) ? 1 : 0;
}