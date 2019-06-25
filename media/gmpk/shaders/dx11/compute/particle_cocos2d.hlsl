static const int GM_MOTION_MODE_FREE = 0;
static const int GM_MOTION_MODE_RELATIVE = 1;

struct gravity_t
{
    float4 initialVelocity;
    float radialAcceleration;
    float tangentialAcceleration;
    float __padding__[2];
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
    float4 color;
    float4 deltaColor;
    float4 position;
    float4 startPosition;
    float4 changePosition;
    float4 velocity;
    float size;
    float currentSize;
    float deltaSize;
    float rotation;
    float deltaRotation;
    float remainingLife;
    float __padding__[2];

    gravity_t gravityModeData;
    radius_t radiusModeData;
};

cbuffer Constant : register(b0)
{
    float4 emitterPosition;
    float4 gravity;
    float4 rotationAxis;
    float dt;
    int motionMode;
};

float4x4 rotate(float angle, float3 axis)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return float4x4
                (oc * axis.x * axis.x + c,           oc * axis.x * axis.y + axis.z * s,  oc * axis.z * axis.x - axis.y * s,  0.0,
                 oc * axis.x * axis.y - axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z + axis.x * s,  0.0,
                 oc * axis.z * axis.x + axis.y * s,  oc * axis.y * axis.z - axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                 0.0,                                0.0,                                0.0,                                1.0);
}

StructuredBuffer<particle_t> particlesIn : register(t0);
RWStructuredBuffer<particle_t> particles : register(u0);

[numthreads(1, 1, 1)]
void gravity_main(uint3 DTid : SV_DispatchThreadID)
{
    uint gid = DTid.x;

    particles[gid] = particlesIn[gid];
    particles[gid].remainingLife -= dt;
    if (particles[gid].remainingLife > 0)
    {
        float3 offset = float3(0, 0, 0);
        float3 radial = float3(0, 0, 0);
        float3 tangential = float3(0, 0, 0);
        if (abs(particles[gid].changePosition.x) > .01f &&
            abs(particles[gid].changePosition.y) > .01f &&
            abs(particles[gid].changePosition.z) > .01f)
        {
            radial = normalize(particles[gid].gravityModeData.initialVelocity.xyz);
        }
        tangential = radial;
        radial *= particles[gid].gravityModeData.initialVelocity.xyz;

        tangential.xyz = float3(-tangential.y, tangential.x, tangential.z);
        tangential *= particles[gid].gravityModeData.tangentialAcceleration;

        offset = (radial + tangential + gravity.xyz) * dt;

        particles[gid].gravityModeData.initialVelocity.xyz += offset;
        particles[gid].changePosition.xyz += particles[gid].gravityModeData.initialVelocity.xyz * dt;
        particles[gid].color += particles[gid].deltaColor * dt;
        particles[gid].size = max(0, particles[gid].size + particles[gid].deltaSize * dt);
        particles[gid].rotation += particles[gid].deltaRotation * dt;

        if (motionMode == GM_MOTION_MODE_RELATIVE)
            particles[gid].position.xyz = particles[gid].changePosition.xyz + emitterPosition.xyz - particles[gid].startPosition.xyz;
        else
            particles[gid].position.xyz = particles[gid].changePosition.xyz;
    }
}

[numthreads(1, 1, 1)]
void radial_main(uint3 DTid : SV_DispatchThreadID)
{
    uint gid = DTid.x;

    particles[gid] = particlesIn[gid];
    particles[gid].remainingLife -= dt;
    if (particles[gid].remainingLife > 0)
    {
        particles[gid].radiusModeData.angle += particles[gid].radiusModeData.degressPerSecond * dt;
        particles[gid].radiusModeData.radius += particles[gid].radiusModeData.deltaRadius * dt;
        float4x4 rotation = rotate(particles[gid].radiusModeData.angle, rotationAxis.xyz);
        particles[gid].changePosition.xyz = particles[gid].radiusModeData.radius * mul(float4(0, 1, 0, 1), rotation).xyz;

        if (motionMode == GM_MOTION_MODE_RELATIVE)
            particles[gid].position.xyz = particles[gid].changePosition.xyz + particles[gid].startPosition.xyz;
        else
            particles[gid].position.xyz = particles[gid].changePosition.xyz + emitterPosition.xyz;

        particles[gid].color += particles[gid].deltaColor * dt;
        particles[gid].size = max(0, particles[gid].size + particles[gid].deltaSize * dt);
        particles[gid].rotation += particles[gid].deltaRotation * dt;
    }
}