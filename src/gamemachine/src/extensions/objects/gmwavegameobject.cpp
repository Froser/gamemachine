#include "stdafx.h"
#include "gmwavegameobject.h"
#include <gmutilities.h>
#include <gmgl/shader_constants.h>
#if GM_USE_DX11
#include <gmdxincludes.h>
#endif

#define getVertexIndex(x, y) ((x) + (y) * (sliceM + 1))
#define __L(txt) L ## txt
#define _L(txt) __L(txt)
#define WAVE_DESCRIPTION "GM_Ext_Wave_WaveDescriptions"
#define STEEPNESS "Steepness"
#define AMPLITUDE "Amplitude"
#define DIRECTION "Direction"
#define SPEED "Speed"
#define WAVELENGTH "WaveLength"

struct GMWaveDescriptionStrings
{
	GMString steepness;
	GMString amplitude;
	GMString direction;
	GMString speed;
	GMString waveLength;
};

namespace
{
	GMRenderTechniqueID s_techId;

	void calculateNormals(GMVertices& vertices, const Vector<GMWaveDescription>& ds, GMfloat duration)
	{
		for (GMVertex& v : vertices)
		{
			GMfloat x = 0, y = 1, z = 0;
			for (GMint32 i = 0; i < ds.size(); ++i)
			{
				GMfloat wi = 2 / ds[i].waveLength;
				GMfloat wa = wi * ds[i].amplitude;
				GMfloat phi = ds[i].speed * wi;
				GMfloat rad = wi * Dot(ds[i].direction, GMVec3(v.positions[0], 0, v.positions[2])) + phi * duration;
				GMfloat c = Cos(rad);
				GMfloat s = Sin(rad);
				GMfloat Qi = ds[i].steepness / (ds[i].amplitude * wi * ds.size());
				x -= ds[i].direction.getX() * wa * c;
				y -= Qi * wa * s;
				z -= ds[i].direction.getZ() * wa * c;
			}

			GMVec3 n = Normalize(GMVec3(x, y, z));
			v.normals = { n.getX(), n.getY(), n.getZ() };
		}
	}

	void calculateTangent(GMVertices& vertices, const Vector<GMWaveDescription>& ds, GMfloat duration)
	{
		for (GMVertex& v : vertices)
		{
			GMfloat x = 0, y = 0, z = 1;
			for (GMint32 i = 0; i < ds.size(); ++i)
			{
				GMfloat wi = 2 / ds[i].waveLength;
				GMfloat wa = wi * ds[i].amplitude;
				GMfloat phi = ds[i].speed * wi;
				GMfloat rad = wi * Dot(ds[i].direction, GMVec3(v.positions[0], 0, v.positions[2])) + phi * duration;
				GMfloat Qi = ds[i].steepness / (ds[i].amplitude * wi * ds.size());
				GMfloat c = Cos(rad);
				GMfloat s = Sin(rad);
				x -= Qi * ds[i].direction.getX() * ds[i].direction.getY() * wa * s;
				y -= Qi * ds[i].direction.getY() * ds[i].direction.getY() * wa * s;
				z += ds[i].direction.getY() * wa * c;
			}

			GMVec3 n = Normalize(GMVec3(x, y, z));
			v.tangents = { n.getX(), n.getY(), n.getZ() };
		}
	}

	void calculateBitangent(GMVertices& vertices)
	{
		for (GMVertex& v : vertices)
		{
			GMVec3 n = GMVec3(v.normals[0], v.normals[1], v.normals[2]);
			GMVec3 t = GMVec3(v.tangents[0], v.tangents[1], v.tangents[2]);
			GMVec3 b = Cross(n, t);
			v.bitangents = { b.getX(), b.getY(), b.getZ() };
		}
	}

	void createTerrain(
		const GMWaveGameObjectDescription& desc,
		REF GMSceneAsset& scene
	)
	{
		GMfloat x = desc.terrainX;
		GMfloat z = desc.terrainZ;
		const GMfloat x_start = x;
		const GMfloat z_start = z;
		const GMfloat& sliceM = desc.sliceM;
		const GMfloat& sliceN = desc.sliceN;

		GMVertices vertices;
		vertices.reserve((sliceM + 1) * (sliceN + 1));

		const GMfloat dx = desc.terrainLength / sliceM; // 2D横向
		const GMfloat dz = desc.terrainWidth / sliceN; // 2D纵向

		// 先计算顶点坐标
		GMfloat y = 0;
		GMfloat u = 0, v = 0;

		for (GMsize_t i = 0; i < sliceN + 1; ++i)
		{
			for (GMsize_t j = 0; j < sliceM + 1; ++j)
			{
				GMfloat x_distance = x - x_start;
				y = 0;

				u = (x_distance) / desc.textureLength;
				v = (z - z_start) / desc.textureHeight;

				GMVertex vert = { { x, y, z },{ 0, 0, 0 },{ u, v } };
				vertices.push_back(std::move(vert));
				x += dx;
			}

			z += dz;
			x = x_start;
		}

		// 顶点数据创建完毕
		GMModel* m = new GMModel();
		GMPart* part = new GMPart(m);
		part->swap(vertices);

		// 接下来创建indices
		m->setUsageHint(GMUsageHint::DynamicDraw);
		m->setDrawMode(GMModelDrawMode::Index);
		m->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
		for (GMsize_t i = 0; i < sliceN; ++i)
		{
			for (GMsize_t j = 0; j < sliceM; ++j)
			{
				part->index(getVertexIndex(j, i));
				part->index(getVertexIndex(j, i + 1));
				part->index(getVertexIndex(j + 1, i + 1));

				part->index(getVertexIndex(j, i));
				part->index(getVertexIndex(j + 1, i + 1));
				part->index(getVertexIndex(j + 1, i));
			}
		}

		scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
	}

	GMfloat gerstner_x(GMfloat q, const GMWaveDescription& desc, GMVec3 pos, GMfloat rad)
	{
		if (FuzzyCompare(desc.steepness, 0))
			return 0;

		return q * desc.amplitude * desc.direction.getX() * pos.getX() * Cos(rad);
	}

	GMfloat gerstner_y(const GMWaveDescription& desc, GMVec3 pos, GMfloat rad)
	{
		return desc.amplitude * Sin(rad);
	}

	GMfloat gerstner_z(GMfloat q, const GMWaveDescription& desc, GMVec3 pos, GMfloat rad)
	{
		if (FuzzyCompare(q, 0))
			return 0;

		return q * desc.amplitude * desc.direction.getZ() * pos.getZ() * Cos(rad);
	}
}

void GMWaveGameObject::initShader(const IRenderContext* context)
{
	IGraphicEngine* engine = context->getEngine();
	GMRenderTechniques techs;
	GMRenderTechnique vertexTech(GMShaderType::Vertex);
	vertexTech.setCode(
		GMRenderEnvironment::OpenGL,
		L"#version 330\n"
		L"#include \"foundation/foundation.h\"\n"
		L"#include \"foundation/vert_header.h\"\n"
		L"out vec4 _model3d_position_world;"
		L"out vec3 _cubemap_uv;"
		L""
		L"struct GMExt_Wave_WaveDescription"
		L"{"
		L"    float Steepness;"
		L"    float Amplitude;"
		L"    vec3 Direction;"
		L"    float Speed;"
		L"    float WaveLength;"
		L"};"
		L""
		L"const int GM_MaxWaves = 10;"
		L"uniform GMExt_Wave_WaveDescription GM_Ext_Wave_WaveDescriptions[GM_MaxWaves];"
		L"uniform int GM_Ext_Wave_WaveCount = 1;"
		L"uniform float GM_Ext_Wave_Duration = 0;"
		L"\n"
		L"void main()"
		L"{"
		L"    init_layouts();"
		L"    vec4 p = vec4(position.x, 0, position.z, 1);"
		L"    vec3 n = vec3(0, 1, 0);"
		L"    vec3 t = vec3(0, 0, 1);"
		L"    for (int i = 0; i < GM_Ext_Wave_WaveCount; ++i)"
		L"    {"
		L"        float wi = 2.f / GM_Ext_Wave_WaveDescriptions[i].WaveLength;"
		L"        float phi = GM_Ext_Wave_WaveDescriptions[i].Speed * wi;"
		L"        float rad = wi * dot(GM_Ext_Wave_WaveDescriptions[i].Direction.xz, position.xz) + phi * GM_Ext_Wave_Duration;"
		L"        float Qi = GM_Ext_Wave_WaveDescriptions[i].Steepness / (GM_Ext_Wave_WaveDescriptions[i].Amplitude * wi * GM_Ext_Wave_WaveCount);"
		L"        float C = cos(rad);"
		L"        float S = sin(rad);"
		L"        p += vec4(Qi * GM_Ext_Wave_WaveDescriptions[i].Amplitude * GM_Ext_Wave_WaveDescriptions[i].Direction.x * position.x * C,"
		L"                  GM_Ext_Wave_WaveDescriptions[i].Amplitude * S,"
		L"                  Qi * GM_Ext_Wave_WaveDescriptions[i].Amplitude * GM_Ext_Wave_WaveDescriptions[i].Direction.z * position.z * C, 0);"
		L"        float wa = wi * GM_Ext_Wave_WaveDescriptions[i].Amplitude;"
		L"        n.xz -= GM_Ext_Wave_WaveDescriptions[i].Direction.xz * C;"
		L"        n.y -= Qi * wa * S;"
		L"        t.x -= Qi * GM_Ext_Wave_WaveDescriptions[i].Direction.x * GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * S;"
		L"        t.y -= Qi * GM_Ext_Wave_WaveDescriptions[i].Direction.y * GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * S;"
		L"        t.z += GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * C;"
		L"    }"
		L"    _model3d_position_world = GM_WorldMatrix * p;"
		L"    gl_Position = (GM_ProjectionMatrix * GM_ViewMatrix * _model3d_position_world);"
		L"    _normal = vec4(normalize(n), 1);"
		L"    _tangent = vec4(normalize(t), 1);"
		L"    _bitangent = vec4(cross(_normal.xyz, _tangent.xyz), 1);"
		L"}"
	);

	vertexTech.setCode(
		GMRenderEnvironment::DirectX11,
		L"struct GMExt_Wave_WaveDescription\n"
		L"{\n"
		L"    float Steepness;\n"
		L"    float Amplitude;\n"
		L"    float3 Direction;\n"
		L"    float Speed;\n"
		L"    float WaveLength;\n"
		L"};\n"
		L""
		L"static const int GM_MaxWaves = 10;\n"
		L"GMExt_Wave_WaveDescription GM_Ext_Wave_WaveDescriptions[GM_MaxWaves];\n"
		L"int GM_Ext_Wave_WaveCount;\n"
		L"float GM_Ext_Wave_Duration;\n"
		L""
		L"VS_OUTPUT VS_GerstnerWave( VS_INPUT input )\n"
		L"{\n"
		L"    VS_OUTPUT output;\n"
		L"    output.Position = GM_ToFloat4(input.Position);\n"
		L"    float4 p = float4(output.Position.x, 0, output.Position.z, 1);\n"
		L"    float3 n = float3(0, 1, 0);\n"
		L"    float3 t = float3(0, 0, 1);\n"
		L"    for (int i = 0; i < GM_Ext_Wave_WaveCount; ++i)\n"
		L"    {\n"
		L"        float wi = 2.f / GM_Ext_Wave_WaveDescriptions[i].WaveLength;\n"
		L"        float phi = GM_Ext_Wave_WaveDescriptions[i].Speed * wi;\n"
		L"        float rad = wi * dot(GM_Ext_Wave_WaveDescriptions[i].Direction.xz, output.Position.xz) + phi * GM_Ext_Wave_Duration;\n"
		L"        float Qi = GM_Ext_Wave_WaveDescriptions[i].Steepness / (GM_Ext_Wave_WaveDescriptions[i].Amplitude * wi * GM_Ext_Wave_WaveCount);\n"
		L"        float C = cos(rad);\n"
		L"        float S = sin(rad);\n"
		L"        p += float4(Qi * GM_Ext_Wave_WaveDescriptions[i].Amplitude * GM_Ext_Wave_WaveDescriptions[i].Direction.x * output.Position.x * C,\n"
		L"                GM_Ext_Wave_WaveDescriptions[i].Amplitude * S,\n"
		L"                Qi * GM_Ext_Wave_WaveDescriptions[i].Amplitude * GM_Ext_Wave_WaveDescriptions[i].Direction.z * output.Position.z * C, 0);\n"
		L"        float wa = wi * GM_Ext_Wave_WaveDescriptions[i].Amplitude;\n"
		L"        n.xz -= GM_Ext_Wave_WaveDescriptions[i].Direction.xz * C;\n"
		L"        n.y -= Qi * wa * S;\n"
		L"        t.x -= Qi * GM_Ext_Wave_WaveDescriptions[i].Direction.x * GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * S;\n"
		L"        t.y -= Qi * GM_Ext_Wave_WaveDescriptions[i].Direction.y * GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * S;\n"
		L"        t.z += GM_Ext_Wave_WaveDescriptions[i].Direction.y * wa * C;\n"
		L"    }\n"
		L"    output.Position = p;"
		L"    output.Position = mul(output.Position, GM_WorldMatrix);\n"
		L"    output.WorldPos = output.Position;\n"
		L"    output.Position = mul(output.Position, GM_ViewMatrix);\n"
		L"    output.Position = mul(output.Position, GM_ProjectionMatrix);\n"
		L"    output.Normal = GM_ToFloat4(normalize(n));\n"
		L"    output.Texcoord = input.Texcoord;\n"
		L"    output.Tangent = GM_ToFloat4(normalize(t));\n"
		L"    output.Bitangent = GM_ToFloat4(cross(output.Normal.xyz, output.Tangent.xyz));\n"
		L"    output.Lightmap = input.Lightmap;\n"
		L"    output.Color = input.Color;\n"
		L"    output.Z = output.Position.z;\n"
		L"    return output;\n"
		L"}\n"
	);
	techs.addRenderTechnique(vertexTech);
	s_techId = engine->getRenderTechniqueManager()->addRenderTechniques(techs);
}

GMWaveGameObject* GMWaveGameObject::create(const GMWaveGameObjectDescription& desc)
{
	GMWaveGameObject* ret = new GMWaveGameObject();
	GMSceneAsset waveScene;
	createTerrain(desc, waveScene);
	ret->setAsset(waveScene);
	ret->setObjectDescription(desc);

	GMModel* waveModel = waveScene.getScene()->getModels()[0].getModel();
	ret->setVertices(waveModel->getParts()[0]->vertices());
	waveModel->setTechniqueId(s_techId);
	return ret;
}

void GMWaveGameObject::setWaveDescriptions(Vector<GMWaveDescription> desc)
{
	D(d);
	d->waveDescriptions = std::move(desc);
}

void GMWaveGameObject::play()
{
	D(d);
	if (!d->isPlaying)
	{
		d->isPlaying = true;
	}
}

void GMWaveGameObject::stop()
{
	D(d);
	if (d->isPlaying)
	{
		d->isPlaying = false;
	}
}

void GMWaveGameObject::update(GMDuration dt)
{
	D(d);
	D_BASE(db, Base);
	if (d->isPlaying)
		d->duration += dt;

	// 使用CPU进行运算，需要手动update每个顶点
	if (!d->waveModel)
		d->waveModel = db->asset.getScene()->getModels()[0].getModel();

	if (d->acceleration == GMWaveGameObjectHardwareAcceleration::CPU)
	{
		updateEachVertex();
		d->waveModel->setType(GMModelType::Model3D);
	}
	else
	{
		d->waveModel->setType(GMModelType::Custom);
	}
}

void GMWaveGameObject::onRenderShader(GMModel* model, IShaderProgram* shaderProgram) const
{
	D(d);
	if (d->acceleration == GMWaveGameObjectHardwareAcceleration::GPU)
	{
		static Vector<GMWaveDescriptionStrings> s_waveDescriptionStrings;
		static const GMString s_waveCount = L"GM_Ext_Wave_WaveCount";
		static const GMString s_duration = L"GM_Ext_Wave_Duration";
		constexpr GMint32 MAX_WAVES = 10;

		if (GM.getRunningStates().renderEnvironment == GMRenderEnvironment::OpenGL)
		{
			static std::once_flag s_flag;
			std::call_once(s_flag, [MAX_WAVES](Vector<GMWaveDescriptionStrings>& strings) {
				strings.resize(MAX_WAVES);
				for (GMint32 i = 0; i < MAX_WAVES; ++i)
				{
					GMString strIdx = GMString(i);
					strings[i].steepness = _L(WAVE_DESCRIPTION) L"[" + strIdx + L"]." _L(STEEPNESS);
					strings[i].amplitude = _L(WAVE_DESCRIPTION) L"[" + strIdx + L"]." _L(AMPLITUDE);
					strings[i].direction = _L(WAVE_DESCRIPTION) L"[" + strIdx + L"]." _L(DIRECTION);
					strings[i].speed = _L(WAVE_DESCRIPTION) L"[" + strIdx + L"]." _L(SPEED);
					strings[i].waveLength = _L(WAVE_DESCRIPTION) L"[" + strIdx + L"]." _L(WAVELENGTH);
				}
			}, s_waveDescriptionStrings);

			// 传递参数
			GMsize_t prog = verifyIndicesContainer(d->globalIndices, shaderProgram);
			verifyIndicesContainer(d->waveIndices, shaderProgram);
			GMint32 waveCount = gm_sizet_to_int(d->waveDescriptions.size());
			if (d->waveIndices[prog].size() <= waveCount)
				d->waveIndices[prog].resize(waveCount + 1);

			shaderProgram->setInt(getVariableIndex(shaderProgram, d->globalIndices[prog].waveCount, s_waveCount), waveCount);
			shaderProgram->setFloat(getVariableIndex(shaderProgram, d->globalIndices[prog].duration, s_duration), d->duration);
			for (GMint32 i = 0; i < waveCount; ++i)
			{
				shaderProgram->setFloat(
					getVariableIndex(shaderProgram, d->waveIndices[prog][i].steepness, s_waveDescriptionStrings[i].steepness),
					d->waveDescriptions[i].steepness);

				shaderProgram->setFloat(
					getVariableIndex(shaderProgram, d->waveIndices[prog][i].amplitude, s_waveDescriptionStrings[i].amplitude),
					d->waveDescriptions[i].amplitude);

				shaderProgram->setVec3(
					getVariableIndex(shaderProgram, d->waveIndices[prog][i].direction, s_waveDescriptionStrings[i].direction),
					ValuePointer(d->waveDescriptions[i].direction));

				shaderProgram->setFloat(
					getVariableIndex(shaderProgram, d->waveIndices[prog][i].speed, s_waveDescriptionStrings[i].speed),
					d->waveDescriptions[i].speed);

				shaderProgram->setFloat(
					getVariableIndex(shaderProgram, d->waveIndices[prog][i].waveLength, s_waveDescriptionStrings[i].waveLength),
					d->waveDescriptions[i].waveLength);
			}
		}
		else
		{
#if GM_USE_DX11
			static std::once_flag s_flag;
			std::call_once(s_flag, [d, shaderProgram]() {
				d->dxWaveEffects.waveCount = shaderProgram->getIndex(s_waveCount);
				d->dxWaveEffects.duration = shaderProgram->getIndex(s_duration);
			});
			GMint32 waveCount = gm_sizet_to_int(d->waveDescriptions.size());
			shaderProgram->setInt(d->dxWaveEffects.waveCount, waveCount);
			shaderProgram->setFloat(d->dxWaveEffects.duration, d->duration);

			GMComPtr<ID3DX11Effect> effect;
			shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&effect);
			for (GMint32 i = 0; i < waveCount; ++i)
			{
				GM_ASSERT(effect);
				auto descriptions = effect->GetVariableByName(WAVE_DESCRIPTION);
				GM_ASSERT(descriptions->IsValid());
				auto description = descriptions->GetElement(i);
				GM_ASSERT(description->IsValid());
				GM_DX_HR(description->GetMemberByName(STEEPNESS)->AsScalar()->SetFloat(d->waveDescriptions[i].steepness));
				GM_DX_HR(description->GetMemberByName(AMPLITUDE)->AsScalar()->SetFloat(d->waveDescriptions[i].amplitude));
				GM_DX_HR(description->GetMemberByName(DIRECTION)->AsVector()->SetFloatVector(ValuePointer(d->waveDescriptions[i].direction)));
				GM_DX_HR(description->GetMemberByName(SPEED)->AsScalar()->SetFloat(d->waveDescriptions[i].speed));
				GM_DX_HR(description->GetMemberByName(WAVELENGTH)->AsScalar()->SetFloat(d->waveDescriptions[i].waveLength));
			}
#else
			GM_ASSERT(false);
#endif
		}
	}
}

void GMWaveGameObject::updateEachVertex()
{
	D(d);
	D_BASE(db, Base);
	if (d->isPlaying)
	{
		GMVertices vertices = d->vertices;
		calculateNormals(vertices, d->waveDescriptions, d->duration);
		calculateTangent(vertices, d->waveDescriptions, d->duration);
		calculateBitangent(vertices);
		for (GMVertex& vertex : vertices)
		{
			GMfloat gerstner_x_sum = 0;
			GMfloat gerstner_y_sum = 0;
			GMfloat gerstner_z_sum = 0;
			GMVec3 pos = { vertex.positions[0], vertex.positions[1], vertex.positions[2] };

			for (GMsize_t i = 0; i < d->waveDescriptions.size(); ++i)
			{
				GMfloat wi = 2 / d->waveDescriptions[i].waveLength;
				GMfloat phi = d->waveDescriptions[i].speed * wi;
				GMfloat rad = wi * (d->waveDescriptions[i].direction.getX() * pos.getX() + d->waveDescriptions[i].direction.getZ() * pos.getZ()) + phi * d->duration;
				GMfloat Qi = d->waveDescriptions[i].steepness / (d->waveDescriptions[i].amplitude * wi * d->waveDescriptions.size());
				gerstner_x_sum += gerstner_x(Qi, d->waveDescriptions[i], pos, rad);
				gerstner_y_sum += gerstner_y(d->waveDescriptions[i], pos, rad);
				gerstner_z_sum += gerstner_z(Qi, d->waveDescriptions[i], pos, rad);
			}

			vertex.positions = {
				vertex.positions[0] + gerstner_x_sum,
				gerstner_y_sum,
				vertex.positions[2] + gerstner_z_sum
			};
		}

		GMModelDataProxy* proxy = getModel()->getModelDataProxy();
		proxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
		void* ptr = proxy->getBuffer();
		GMsize_t sz = sizeof(GMVertex) * vertices.size();
		memcpy_s(ptr, sz, vertices.data(), sz);
		proxy->endUpdateBuffer();
	}
}

void GMWaveGameObject::setVertices(const GMVertices& vertices)
{
	D(d);
	d->vertices = vertices;
}
