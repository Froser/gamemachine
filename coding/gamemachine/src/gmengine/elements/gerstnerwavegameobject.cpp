#include "stdafx.h"
#include "gerstnerwavegameobject.h"
#include "gmengine/elements/gameworld.h"

#define STRIP_COUNT		80
#define STRIP_LENGTH	50
#define DATA_LENGTH		STRIP_LENGTH*2*(STRIP_COUNT-1)
#define WAVE_COUNT		6
static GLfloat pt_strip[STRIP_COUNT*STRIP_LENGTH * 4] = { 0 };
static GLfloat pt_normal[STRIP_COUNT*STRIP_LENGTH * 4] = { 0 };
static GLfloat vertex_data[DATA_LENGTH * 4] = { 0 };
static GLfloat normal_data[DATA_LENGTH * 4] = { 0 };

//wave_length, wave_height, wave_dir, wave_speed, wave_start.x, wave_start.y
static const GLfloat wave_para[6][6] = {
	{ 1.6,	0.12,	0.9,	0.06,	0.0,	0.0 },
	{ 1.3,	0.1,	1.14,	0.09,	0.0,	0.0 },
	{ 0.2,	0.01,	0.8,	0.08,	0.0,	0.0 },
	{ 0.18,	0.008,	1.05,	0.1,	0.0,	0.0 },
	{ 0.23,	0.005,	1.15,	0.09,	0.0,	0.0 },
	{ 0.12,	0.003,	0.97,	0.14,	0.0,	0.0 }
};

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

#define START_X		-4.0
#define START_Y		-2.5
#define START_Z		0
#define LENGTH_X	0.1
#define LENGTH_Y	0.1
#define HEIGHT_SCALE	1.6

static struct {
	GLfloat time;
	GLfloat wave_length[WAVE_COUNT],
		wave_height[WAVE_COUNT],
		wave_dir[WAVE_COUNT],
		wave_speed[WAVE_COUNT],
		wave_start[WAVE_COUNT * 2];
} values;

GerstnerWaveGameObject::GerstnerWaveGameObject(const Material& material)
	: HallucinationGameObject(nullptr)
	, m_material(material)
{
	initAll();
}

void GerstnerWaveGameObject::initAll()
{
	D(d);
	initWave();
	m_lastTick = d.world ? d.world->getElapsed() : 0;
}

void GerstnerWaveGameObject::initWave()
{
	values.time = 0.0;
	for (int w = 0; w < WAVE_COUNT; w++)
	{
		values.wave_length[w] = wave_para[w][0];
		values.wave_height[w] = wave_para[w][1];
		values.wave_dir[w] = wave_para[w][2];
		values.wave_speed[w] = wave_para[w][3];
		values.wave_start[w * 2] = wave_para[w][4];
		values.wave_start[w * 2 + 1] = wave_para[w][5];
	}

	//Initialize pt_strip[]
	int index = 0;
	for (int i = 0; i < STRIP_COUNT; i++)
	{
		for (int j = 0; j < STRIP_LENGTH; j++)
		{
			pt_strip[index] = START_X + i*LENGTH_X;
			pt_strip[index + 1] = START_Y + j*LENGTH_Y;
			pt_strip[index + 3] = 1.f;
			index += 4;
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

static int normalizeF(float in[], float out[], int count)
{
	int t = 0;
	float l = 0.0;

	if (count <= 0.0) {
		printf("normalizeF(): Number of dimensions should be larger than zero.\n");
		return 1;
	}
	while (t < count && in[t]<0.0000001 && in[t]>-0.0000001) {
		t++;
	}
	if (t == count) {
		printf("normalizeF(): The input vector is too small.\n");
		return 1;
	}
	for (t = 0; t < count; t++)
		l += in[t] * in[t];
	if (l < 0.0000001) {
		l = 0.0;
		for (t = 0; t < count; t++)
			in[t] *= 10000.0;
		for (t = 0; t < count; t++)
			l += in[t] * in[t];
	}
	l = sqrt(l);
	for (t = 0; t < count; t++)
		out[t] /= l;

	return 0;
}

void GerstnerWaveGameObject::calcWave()
{
	//Calculate pt_strip[z], poly_normal[] and pt_normal[]
	int index = 0;
	float d, wave;
	for (int i = 0; i < STRIP_COUNT; i++)
	{
		for (int j = 0; j < STRIP_LENGTH; j++)
		{
			wave = 0.0;
			for (int w = 0; w < WAVE_COUNT; w++) {
				d = (pt_strip[index] - values.wave_start[w * 2] + (pt_strip[index + 1] - values.wave_start[w * 2 + 1]) * tan(values.wave_dir[w])) * cos(values.wave_dir[w]);
				if (gerstner_sort[w] == 1) {
					wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_a);
				}
				else {
					wave += values.wave_height[w] - gerstnerZ(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_b);
				}
			}
			pt_strip[index + 2] = START_Z + wave*HEIGHT_SCALE;
			index += 4;
		}
	}

	index = 0;
	for (int i = 0; i < STRIP_COUNT; i++)
	{
		for (int j = 0; j < STRIP_LENGTH; j++)
		{
			int p0 = index - STRIP_LENGTH * 3, p1 = index + 3, p2 = index + STRIP_LENGTH * 3, p3 = index - 3;
			float xa, ya, za, xb, yb, zb;
			if (i > 0) {
				if (j > 0) {
					xa = pt_strip[p0] - pt_strip[index], ya = pt_strip[p0 + 1] - pt_strip[index + 1], za = pt_strip[p0 + 2] - pt_strip[index + 2];
					xb = pt_strip[p3] - pt_strip[index], yb = pt_strip[p3 + 1] - pt_strip[index + 1], zb = pt_strip[p3 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
					pt_normal[index + 3] = 1.0f;
				}
				if (j < STRIP_LENGTH - 1) {
					xa = pt_strip[p1] - pt_strip[index], ya = pt_strip[p1 + 1] - pt_strip[index + 1], za = pt_strip[p1 + 2] - pt_strip[index + 2];
					xb = pt_strip[p0] - pt_strip[index], yb = pt_strip[p0 + 1] - pt_strip[index + 1], zb = pt_strip[p0 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
					pt_normal[index + 3] = 1.0f;
				}
			}
			if (i < STRIP_COUNT - 1) {
				if (j > 0) {
					xa = pt_strip[p3] - pt_strip[index], ya = pt_strip[p3 + 1] - pt_strip[index + 1], za = pt_strip[p3 + 2] - pt_strip[index + 2];
					xb = pt_strip[p2] - pt_strip[index], yb = pt_strip[p2 + 1] - pt_strip[index + 1], zb = pt_strip[p2 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
					pt_normal[index + 3] = 1.0f;
				}
				if (j < STRIP_LENGTH - 1) {
					xa = pt_strip[p2] - pt_strip[index], ya = pt_strip[p2 + 1] - pt_strip[index + 1], za = pt_strip[p2 + 2] - pt_strip[index + 2];
					xb = pt_strip[p1] - pt_strip[index], yb = pt_strip[p1 + 1] - pt_strip[index + 1], zb = pt_strip[p1 + 2] - pt_strip[index + 2];
					pt_normal[index] += ya*zb - yb*za;
					pt_normal[index + 1] += xb*za - xa*zb;
					pt_normal[index + 2] += xa*yb - xb*ya;
					pt_normal[index + 3] = 1.0f;
				}
			}
			if (normalizeF(&pt_normal[index], &pt_normal[index], 3))
				printf("%d\t%d\n", index / 3 / STRIP_LENGTH, (index / 3) % STRIP_LENGTH);

			index += 4;
		}
	}

	//Calculate vertex_data[] according to pt_strip[], and normal_data[] according to pt_normal[]
	int pt;
	for (int c = 0; c < (STRIP_COUNT - 1); c++)
	{
		for (int l = 0; l < 2 * STRIP_LENGTH; l++)
		{
			if (l % 2 == 1) {
				pt = c*STRIP_LENGTH + l / 2;
			}
			else {
				pt = c*STRIP_LENGTH + l / 2 + STRIP_LENGTH;
			}
			index = STRIP_LENGTH * 2 * c + l;
			for (int i = 0; i < 4; i++) {
				vertex_data[index * 4 + i] = pt_strip[pt * 4 + i];
				normal_data[index * 4 + i] = pt_normal[pt * 4 + i];
			}
		}
	}
}

void push(std::vector<Object::DataType>& v, GMfloat* f, GMuint cnt)
{
	for (GMuint i = 0; i < cnt; i++)
	{
		v.push_back(f[i]);
	}
}

Object* GerstnerWaveGameObject::createCoreObject()
{
	Object* newObject = new Object();
	newObject->setArrangementMode(Object::Triangle_Strip);

	calcWave();
	push(newObject->vertices(), vertex_data, DATA_LENGTH * 4);
	push(newObject->normals(), normal_data, DATA_LENGTH * 4);

	for (int c = 0; c < (STRIP_COUNT - 1); c++)
	{
		Component* component = new Component(STRIP_LENGTH * 2);
		memcpy(&component->getMaterial(), &m_material, sizeof(Material));
		component->setOffset(STRIP_LENGTH * 2 * c);
		newObject->appendComponent(component, STRIP_LENGTH * 2);
	}

	return newObject;
}

void GerstnerWaveGameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	GMfloat tick = d.world->getElapsed();
	{
		values.time += (tick - m_lastTick) * 2;
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