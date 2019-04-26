#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(std140, binding = 0) uniform Constant
{
    mat4 billboardRotation;
    int ignoreZ;
};

struct gravity_t
{
    vec3 initialVelocity;
    float _padding_; // initialVelocity's placeholder
    float radialAcceleration;
    float tangentialAcceleration;
};

struct radius_t
{
    float angle;
    float degressPerSecond;
    float radius;
    float deltaRadius;
};

struct particle_t
{
    vec4 color;
    vec4 deltaColor;
    vec3 position;
    vec3 startPosition;
    vec3 changePosition;
    vec3 velocity;
    float _padding_; // velocity's placeholder
    float size;
    float currentSize;
    float deltaSize;
    float rotation;
    float deltaRotation;
    float remainingLife;

    gravity_t gravityModeData;
    radius_t radiusModeData;
};

layout(std430, binding = 1) buffer Particle
{
    particle_t particles[];
};

struct vertex_t
{
    float positions[3];
    float normals[3];
    float texcoords[2];
    float tangents[3];
    float bitangents[3];
    float lightmaps[2];
    float colors[4];
    int boneIDs[4];
    float weights[4];
};

layout(std430, binding = 2) buffer Vertex
{
    vertex_t vertex[];
};

mat4 rotate(float angle, vec3 axis)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

mat4 translate(vec3 pos)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(pos.x, pos.y, pos.z, 1.0)
    );
}

void main(void)
{
    uint gid = gl_GlobalInvocationID.x;

    vec2 texcoord[4] = {
        vec2(0, 1),
        vec2(0, 0),
        vec2(1, 1),
        vec2(1, 0)
    };

    float z = (ignoreZ != 0) ? 0 : particles[gid].position.z;
    float halfExt = particles[gid].size / 2.f;
    vec4 raw[4] = {
        vec4(particles[gid].position.x - halfExt, particles[gid].position.y - halfExt, z, 1),
        vec4(particles[gid].position.x - halfExt, particles[gid].position.y + halfExt, z, 1),
        vec4(particles[gid].position.x + halfExt, particles[gid].position.y - halfExt, z, 1),
        vec4(particles[gid].position.x + halfExt, particles[gid].position.y + halfExt, z, 1),
    };

    mat4 rotation = rotate(particles[gid].rotation, vec3(0, 0, 1));
    vec4 transformed[4] = {
        rotation * raw[0],
        rotation * raw[1],
        rotation * raw[2],
        rotation * raw[3],
    };

    // 使用billboard效果
    if (ignoreZ != 0)
    {
        mat4 transToOrigin = translate(-particles[gid].position);
        mat4 transToPos = translate(particles[gid].position);
        transformed[0] = transToPos * billboardRotation * transToOrigin * raw[0];
        transformed[1] = transToPos * billboardRotation * transToOrigin * raw[1];
        transformed[2] = transToPos * billboardRotation * transToOrigin * raw[2];
        transformed[3] = transToPos * billboardRotation * transToOrigin * raw[3];
    }

    // 排列方式：
    // 1   | 1 3
    // 0 2 |   2
    // (0, 1, 2), (2, 1, 3)
    vec3 vertices[4] = {
        transformed[0].xyz,
        transformed[1].xyz,
        transformed[2].xyz,
        transformed[3].xyz,
    };

    uint vertexOffset = gid * 6; // 每个particle用6个顶点

    // vertex 0
    vertex[vertexOffset].positions[0] = vertices[0].x;
    vertex[vertexOffset].positions[1] = vertices[0].y;
    vertex[vertexOffset].positions[2] = vertices[0].z;
    vertex[vertexOffset].texcoords[0] = texcoord[0].x;
    vertex[vertexOffset].texcoords[1] = texcoord[0].y;

    // vertex 1
    vertex[vertexOffset + 1].positions[0] = vertices[1].x;
    vertex[vertexOffset + 1].positions[1] = vertices[1].y;
    vertex[vertexOffset + 1].positions[2] = vertices[1].z;
    vertex[vertexOffset + 1].texcoords[0] = texcoord[1].x;
    vertex[vertexOffset + 1].texcoords[1] = texcoord[1].y;

    // vertex 2
    vertex[vertexOffset + 2].positions[0] = vertices[2].x;
    vertex[vertexOffset + 2].positions[1] = vertices[2].y;
    vertex[vertexOffset + 2].positions[2] = vertices[2].z;
    vertex[vertexOffset + 2].texcoords[0] = texcoord[2].x;
    vertex[vertexOffset + 2].texcoords[1] = texcoord[2].y;

    // vertex 3
    vertex[vertexOffset + 3].positions[0] = vertices[2].x;
    vertex[vertexOffset + 3].positions[1] = vertices[2].y;
    vertex[vertexOffset + 3].positions[2] = vertices[2].z;
    vertex[vertexOffset + 3].texcoords[0] = texcoord[2].x;
    vertex[vertexOffset + 3].texcoords[1] = texcoord[2].y;

    // vertex 4
    vertex[vertexOffset + 4].positions[0] = vertices[1].x;
    vertex[vertexOffset + 4].positions[1] = vertices[1].y;
    vertex[vertexOffset + 4].positions[2] = vertices[1].z;
    vertex[vertexOffset + 4].texcoords[0] = texcoord[1].x;
    vertex[vertexOffset + 4].texcoords[1] = texcoord[1].y;

    // vertex 5
    vertex[vertexOffset + 5].positions[0] = vertices[3].x;
    vertex[vertexOffset + 5].positions[1] = vertices[3].y;
    vertex[vertexOffset + 5].positions[2] = vertices[3].z;
    vertex[vertexOffset + 5].texcoords[0] = texcoord[3].x;
    vertex[vertexOffset + 5].texcoords[1] = texcoord[3].y;

    // common
    for (int i = 0; i < 6; ++i)
    {
        vertex[vertexOffset + i].normals[0] = vertex[vertexOffset + i].normals[1] = 0.f;
        vertex[vertexOffset + i].normals[2] = -1.f;
        vertex[vertexOffset + i].tangents[0] = vertex[vertexOffset + i].tangents[1] = vertex[vertexOffset + i].tangents[2] = 
        vertex[vertexOffset + i].bitangents[0] = vertex[vertexOffset + i].bitangents[1] = vertex[vertexOffset + i].bitangents[2] = 
        vertex[vertexOffset + i].lightmaps[0] = vertex[vertexOffset + i].lightmaps[1] = 
        vertex[vertexOffset + i].weights[0] = vertex[vertexOffset + i].weights[1] = vertex[vertexOffset + i].weights[2] = vertex[vertexOffset + i].weights[3] = 0.f;
        vertex[vertexOffset + i].boneIDs[0] = vertex[vertexOffset + i].boneIDs[1] = vertex[vertexOffset + i].boneIDs[2] = vertex[vertexOffset + i].boneIDs[3] = 0;

        vertex[vertexOffset + i].colors[0] = particles[gid].color.x;
        vertex[vertexOffset + i].colors[1] = particles[gid].color.y;
        vertex[vertexOffset + i].colors[2] = particles[gid].color.z;
        vertex[vertexOffset + i].colors[3] = particles[gid].color.w;
    }
}