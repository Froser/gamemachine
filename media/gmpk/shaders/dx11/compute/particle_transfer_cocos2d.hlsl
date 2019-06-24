cbuffer Constant : register(b0)
{
	float4x4 billboardRotation;
	int ignoreZ;
	float __padding[3];
};

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

StructuredBuffer<particle_t> particles : register(t0);
RWStructuredBuffer<vertex_t> vertex : register(u0);

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

float4x4 translate(float3 pos)
{
    return float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(pos.x, pos.y, pos.z, 1.0)
    );
}

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint gid = DTid.x;

	float2 texcoord[4] = {
		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0)
	};

    // ignoreZ如果为1，表示忽略z坐标，这是个2D上的粒子渲染
	float z = (ignoreZ != 0) ? 0 : particles[gid].position.z;
	float halfExt = particles[gid].size / 2.f;
	float4 raw[4] = {
		float4(particles[gid].position.x - halfExt, particles[gid].position.y - halfExt, z, 1),
		float4(particles[gid].position.x - halfExt, particles[gid].position.y + halfExt, z, 1),
		float4(particles[gid].position.x + halfExt, particles[gid].position.y - halfExt, z, 1),
		float4(particles[gid].position.x + halfExt, particles[gid].position.y + halfExt, z, 1),
	};

	float4x4 rotation = rotate(particles[gid].rotation, float3(0, 0, 1));
	float4 transformed[4] = {
		mul(raw[0], rotation),
		mul(raw[1], rotation),
		mul(raw[2], rotation),
		mul(raw[3], rotation),
	};

	// 使用billboard效果
	if (ignoreZ == 0)
	{
		float4x4 transToOrigin = translate(-particles[gid].position.xyz);
		float4x4 transToPos = translate(particles[gid].position.xyz);
		float4x4 tBillboard = transpose(billboardRotation);
		transformed[0] = mul(mul(mul(raw[0], transToOrigin), tBillboard), transToPos);
		transformed[1] = mul(mul(mul(raw[1], transToOrigin), tBillboard), transToPos);
		transformed[2] = mul(mul(mul(raw[2], transToOrigin), tBillboard), transToPos);
		transformed[3] = mul(mul(mul(raw[3], transToOrigin), tBillboard), transToPos);
	}

	// 排列方式：
	// 1   | 1 3
	// 0 2 |   2
	// (0, 1, 2), (2, 1, 3)
	float3 vertices[4] = {
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