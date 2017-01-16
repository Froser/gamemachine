#include "stdafx.h"
#include "gerstnerwavegameobject.h"
#include "gmengine/elements/gameworld.h"

static const GLfloat gerstner_pt_a[22] = {
	0.0,0.0, 41.8,1.4, 77.5,5.2, 107.6,10.9,
	132.4,17.7, 152.3,25.0, 167.9,32.4, 179.8,39.2,
	188.6,44.8, 195.0,48.5, 200.0,50.0
};
static const GLfloat gerstner_pt_b[22] = {
	0.0,0.0, 27.7,1.4, 52.9,5.2, 75.9,10.8,
	97.2,17.6, 116.8,25.0, 135.1,32.4, 152.4,39.2,
	168.8,44.8, 184.6,48.5, 200.0,50.0
};
static const GLint gerstner_sort[6] = {
	0, 0, 1, 1, 1, 1
};

GerstnerWavesProperties& GerstnerWaveGameObject::defaultProperties()
{
	static GerstnerWaveProperties waves[6] = {
		{ 1.6,	0.12,	0.9,	0.06,	0.0,	0.0 },
		{ 1.3,	0.1,	1.14,	0.09,	0.0,	0.0 },
		{ 0.2,	0.01,	0.8,	0.08,	0.0,	0.0 },
		{ 0.18,	0.008,	1.05,	0.1,	0.0,	0.0 },
		{ 0.23,	0.005,	1.15,	0.09,	0.0,	0.0 },
		{ 0.12,	0.003,	0.97,	0.14,	0.0,	0.0 }
	};

	static GerstnerWavesProperties properties = {
		0.1f,
		0.1f,
		50,
		50,
		6,
		1.2,
		waves
	};

	return properties;
}

GerstnerWaveGameObject::GerstnerWaveGameObject(const Material& material, const GerstnerWavesProperties& props)
	: HallucinationGameObject(nullptr)
	, m_material(material)
	, m_magnification(1)
{
	copyProperties(props);
}

GerstnerWaveGameObject::GerstnerWaveGameObject(const Material& material, GMfloat magnification, const GerstnerWavesProperties& props)
	: HallucinationGameObject(nullptr)
	, m_material(material)
	, m_magnification(magnification <= 0 ? 1 : magnification)
{
	copyProperties(props);
}


GerstnerWaveGameObject::~GerstnerWaveGameObject()
{
	delete[] m_props.waves;
}

void GerstnerWaveGameObject::copyProperties(const GerstnerWavesProperties& props)
{
	m_props = props;
	GerstnerWaveProperties* waves = new GerstnerWaveProperties[m_props.wavesCount];
	for (GMuint i = 0; i < m_props.wavesCount; i++)
	{
		waves[i] = m_props.waves[i];
	}
	m_props.waves = waves;
}

void GerstnerWaveGameObject::initSize()
{
	m_dataLength = (m_props.stripCount - 1) * m_props.stripLength * 2 * 4;
	m_rawPointsLength = m_props.stripCount * m_props.stripLength * 4;
	m_rawStrips.reserve(m_rawPointsLength);
}

void GerstnerWaveGameObject::init()
{
	D(d);
	initSize();
	initWave();
	m_lastTick = d.world ? d.world->getElapsed() : 0;
}

void GerstnerWaveGameObject::initWave()
{
	D(d);
	btVector3 origin = d.transform.getOrigin();

	for (GMuint i = 0; i < m_props.stripCount; i++)
	{
		for (GMuint j = 0; j < m_props.stripLength; j++)
		{
			m_rawStrips.push_back(origin[0] + i * m_props.deltaX);
			m_rawStrips.push_back(origin[1] + j * m_props.deltaY);
			m_rawStrips.push_back(0); //随便放一个值，这个是要求的高度
			m_rawStrips.push_back(1);
		}
	}
}

static float gerstnerZ(float w_length, float w_height, float x_in, const GLfloat gerstner[22])
{
	x_in = x_in * 400.0 / w_length;

	while (x_in < 0.0)
		x_in += 400.0;
	while (x_in > 400.0)
		x_in -= 400.0;
	if (x_in > 200.0)
		x_in = 400.0 - x_in;

	int i = 0;
	float yScale = w_height / 50.0;
	while (i < 18 && (x_in < gerstner[i] || x_in >= gerstner[i + 2]))
		i += 2;
	if (x_in == gerstner[i])
		return gerstner[i + 1] * yScale;
	if (x_in > gerstner[i])
		return ((gerstner[i + 3] - gerstner[i + 1]) * (x_in - gerstner[i]) / (gerstner[i + 2] - gerstner[i]) + gerstner[i + 3]) * yScale;
}

void GerstnerWaveGameObject::calcWave(Object* obj, GMfloat elapsed)
{
	D(_d);
	m_rawNormals.resize(m_rawPointsLength);
	obj->uvs().resize(m_dataLength / 2);
	obj->vertices().resize(m_dataLength);
	obj->normals().resize(m_dataLength);
	GMfloat wave = 0.0f;
	GMuint index = 0;
	GMfloat d = 0.0f;
	for (GMuint i = 0; i < m_props.stripCount; i++)
	{
		for (GMuint j = 0; j < m_props.stripLength; j++)
		{
			wave = 0.0f;
			for (GMuint w = 0; w < m_props.wavesCount; w++)
			{
				d = (m_rawStrips[index] - m_props.waves[w].startX + (m_rawStrips[index + 1] - m_props.waves[w].startY) 
					* tan(m_props.waves[w].waveDirection))
					* cos(m_props.waves[w].waveDirection);
				if (gerstner_sort[w] == 1)
					wave += m_props.waves[w].waveAmplitude - gerstnerZ(
						m_props.waves[w].waveLength,
						m_props.waves[w].waveAmplitude,
						d + m_props.waves[w].waveSpeed * elapsed,
						gerstner_pt_a
					);
				else
					wave += m_props.waves[w].waveAmplitude - gerstnerZ(
						m_props.waves[w].waveLength,
						m_props.waves[w].waveAmplitude,
						d + m_props.waves[w].waveSpeed * elapsed,
						gerstner_pt_b
					);
				m_rawStrips[index + 2] = wave * m_props.waveHeightScale + _d.transform.getOrigin()[2];
			}
			index += 4;
		}
	}

	index = 0;
	for (GMuint i = 0; i < m_props.stripCount; i++)
	{
		for (GMuint j = 0; j < m_props.stripLength; j++)
		{
			GMint p0 = index - m_props.stripLength * 4, p1 = index + 4, p2 = index + m_props.stripLength * 4, p3 = index - 4;
			GMfloat xa, ya, za, xb, yb, zb;
			if (i > 0)
			{
				if (j > 0)
				{
					xa = m_rawStrips[p0] - m_rawStrips[index], ya = m_rawStrips[p0 + 1] - m_rawStrips[index + 1], za = m_rawStrips[p0 + 2] - m_rawStrips[index + 2];
					xb = m_rawStrips[p3] - m_rawStrips[index], yb = m_rawStrips[p3 + 1] - m_rawStrips[index + 1], zb = m_rawStrips[p3 + 2] - m_rawStrips[index + 2];
				}
				if (j < m_props.stripLength - 1)
				{
					xa = m_rawStrips[p1] - m_rawStrips[index], ya = m_rawStrips[p1 + 1] - m_rawStrips[index + 1], za = m_rawStrips[p1 + 2] - m_rawStrips[index + 2];
					xb = m_rawStrips[p0] - m_rawStrips[index], yb = m_rawStrips[p0 + 1] - m_rawStrips[index + 1], zb = m_rawStrips[p0 + 2] - m_rawStrips[index + 2];
				}
			}
			if (i < m_props.stripCount - 1)
			{
				if (j > 0)
				{
					xa = m_rawStrips[p3] - m_rawStrips[index], ya = m_rawStrips[p3 + 1] - m_rawStrips[index + 1], za = m_rawStrips[p3 + 2] - m_rawStrips[index + 2];
					xb = m_rawStrips[p2] - m_rawStrips[index], yb = m_rawStrips[p2 + 1] - m_rawStrips[index + 1], zb = m_rawStrips[p2 + 2] - m_rawStrips[index + 2];
				}
				if (j < m_props.stripLength - 1)
				{
					xa = m_rawStrips[p2] - m_rawStrips[index], ya = m_rawStrips[p2 + 1] - m_rawStrips[index + 1], za = m_rawStrips[p2 + 2] - m_rawStrips[index + 2];
					xb = m_rawStrips[p1] - m_rawStrips[index], yb = m_rawStrips[p1 + 1] - m_rawStrips[index + 1], zb = m_rawStrips[p1 + 2] - m_rawStrips[index + 2];
				}
			}

			// normalize
			vmath::vec3 normalized = vmath::normalize(vmath::vec3(ya*zb - yb*za, xb*za - xa*zb, xa*yb - xb*ya));

			m_rawNormals[index] += normalized[0];
			m_rawNormals[index + 1] += normalized[1];
			m_rawNormals[index + 2] += normalized[2];
			m_rawNormals[index + 3] = 1.0f;

			index += 4;
		}
	}

	GMuint pt;
	for (int c = 0; c < (m_props.stripCount - 1); c++)
	{
		for (int l = 0; l < 2 * m_props.stripLength; l++)
		{
			if (l % 2 == 1)
			{
				pt = c * m_props.stripLength + l / 2;
			}
			else
			{
				pt = c * m_props.stripLength + l / 2 + m_props.stripLength;
			}
			index = m_props.stripLength * 2 * c + l;
			for (int i = 0; i < 4; i++) {
				obj->vertices()[index * 4 + i] = m_rawStrips[pt * 4 + i];
				obj->normals()[index * 4 + i] = m_rawNormals[pt * 4 + i];
			}
			GMfloat uv_x = (GMfloat)(pt / m_props.stripLength) * m_magnification / m_props.stripCount;
			GMfloat uv_y = (GMfloat)(pt % m_props.stripLength) * m_magnification / m_props.stripLength;
			obj->uvs()[index * 2] = uv_x;
			obj->uvs()[index * 2 + 1] = uv_y;
		}
	}
}

Object* GerstnerWaveGameObject::createCoreObject()
{
	Object* newObject = new Object();
	newObject->setArrangementMode(Object::Triangle_Strip);
	calcWave(newObject, m_lastTick);

	for (int c = 0; c < (m_props.stripCount - 1); c++)
	{
		Component* component = new Component(m_props.stripLength * 2);
		memcpy(&component->getMaterial(), &m_material, sizeof(Material));
		component->setOffset(m_props.stripLength * 2 * c);
		newObject->appendComponent(component, m_props.stripLength * 2);
	}

	return newObject;
}

void GerstnerWaveGameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	GMfloat tick = d.world->getElapsed();
	{
		m_lastTick = tick;
		ObjectPainter* painter = d.object->getPainter();
		ObjectPainter* newPainter = nullptr;
		Object* newObject = createCoreObject();

		ASSERT(newObject);
		painter->clone(newObject, &newPainter);
		d.object.reset(newObject);
		d.object->setPainter(newPainter);
		newPainter->transfer();
	}
	HallucinationGameObject::getReadyForRender(list);
}