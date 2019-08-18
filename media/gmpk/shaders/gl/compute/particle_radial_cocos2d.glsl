const int GM_MOTION_MODE_FREE = 0;
const int GM_MOTION_MODE_RELATIVE = 1;

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

layout(std430, binding = 0) buffer ParticleIn
{
    particle_t particlesIn[];
};

layout(std140, binding = 1) uniform Constant
{
    vec3 emitterPosition;
    vec3 gravity;
    vec3 rotationAxis;
    float _padding_; // gravity's placeholder
    float dt;
    int motionMode;
};

layout(std430, binding = 2) buffer Particle
{
    particle_t particles[];
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

layout(local_size_x = 1, local_size_y = 1) in;

void main(void)
{
    uint gid = gl_GlobalInvocationID.x;
    
    particles[gid] = particlesIn[gid];
    particles[gid].remainingLife -= dt;
    if (particles[gid].remainingLife > 0)
    {
        particles[gid].radiusModeData.angle += particles[gid].radiusModeData.degressPerSecond * dt;
        particles[gid].radiusModeData.radius += particles[gid].radiusModeData.deltaRadius * dt;
        mat4 rotation = rotate(particles[gid].radiusModeData.angle, rotationAxis);
        particles[gid].changePosition = (particles[gid].radiusModeData.radius * rotation * vec4(0, 1, 0, 1)).xyz;

        if (motionMode == GM_MOTION_MODE_RELATIVE)
            particles[gid].position = particles[gid].changePosition + particles[gid].startPosition;
        else
            particles[gid].position = particles[gid].changePosition + emitterPosition;

        particles[gid].color += particles[gid].deltaColor * dt;
        particles[gid].size = max(0, particles[gid].size + particles[gid].deltaSize * dt);
        particles[gid].rotation += particles[gid].deltaRotation * dt;
    }
}
