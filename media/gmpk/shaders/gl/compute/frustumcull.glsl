// Compute Shader For Frustum Cull
struct vertices_t
{
    vec4 vertices[8];
};

layout(std140, binding = 0) uniform FrustumPlanes
{
    vec4 planes[6]; // //Normal & intercept. X.N + intercept = 0
};

layout(std430, binding = 1) buffer AABB
{
    vertices_t vertices[]; // AABB
};

layout(std430, binding = 2) buffer Result
{
    int visible[];
};

layout (local_size_x = 1, local_size_y = 1) in;

const float EPSILON = 0.01f;
const int POINT_ON_PLANE = 0;
const int POINT_IN_FRONT_OF_PLANE = 1;
const int POINT_BEHIND_PLANE = 2;

int classifyPoint(vec3 pt, vec4 plane)
{
    float distance = dot(pt, plane.xyz) + plane.w;
    if (distance > EPSILON)
        return POINT_IN_FRONT_OF_PLANE;

    if (distance < -EPSILON)
        return POINT_BEHIND_PLANE;

    return POINT_ON_PLANE;
}

bool isBoundingBoxInside(vec4 planes[6], vertices_t vertices)
{
    for (int i = 0; i < 6; ++i)
    {
        //if a point is not behind this plane, try next plane
        if (classifyPoint(vertices.vertices[0].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[1].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[2].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[3].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[4].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[5].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[6].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;
        if (classifyPoint(vertices.vertices[7].xyz, planes[i]) != POINT_BEHIND_PLANE)
            continue;

        //All vertices of the box are behind this plane
        return false;
    }

    return true;
}

void main(void)
{
    uint gid = gl_GlobalInvocationID.x;
    visible[gid] = isBoundingBoxInside(planes, vertices[gid]) ? 1 : 0;;
}
