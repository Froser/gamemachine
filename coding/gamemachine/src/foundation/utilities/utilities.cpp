#include "stdafx.h"
#include "utilities.h"
#include <linearmath.h>
#include "assert.h"
#include "foundation/vector.h"

//GMClock
GMClock::GMClock()
{
	D(d);
	d->fps = 0;
	d->timeScale = 1.f;
}

void GMClock::setTimeScale(GMfloat s)
{
	D(d);
	d->timeScale = s;
}

void GMClock::setPaused(bool b)
{
	D(d);
	d->paused = b;
}

void GMClock::begin()
{
	D(d);
	d->timeCycles = 0;
	d->paused = false;
	d->frequency = highResolutionTimerFrequency();
	d->begin = highResolutionTimer();
	d->frameCount = 0;
	d->lastCycle = 0;
	d->deltaCycles = 0;
}

// 每一帧运行一次update
void GMClock::update()
{
	D(d);
	d->end = highResolutionTimer();
	GMLargeInteger delta = d->end - d->begin;

	if (!d->paused)
	{
		d->deltaCycles = delta * d->timeScale;
		d->timeCycles += d->deltaCycles;
	}
	else
	{
		d->deltaCycles = 0;
	}

	++d->frameCount;
	GMLargeInteger deltaFrameCycle = d->end - d->lastCycle;
	if (deltaFrameCycle > d->frequency)
	{
		d->fps = d->frameCount / cycleToSecond(deltaFrameCycle);
		d->frameCount = 0;
		d->lastCycle = d->end;
	}

	d->begin = d->end;
}

GMfloat GMClock::elapsedFromStart()
{
	D(d);
	GMLargeInteger now = highResolutionTimer();
	return cycleToSecond(now - d->begin);
}

GMfloat GMClock::getFps()
{
	D(d);
	return d->fps;
}

GMfloat GMClock::getTime()
{
	D(d);
	return cycleToSecond(d->timeCycles);
}

GMfloat GMClock::evaluateDeltaTime()
{
	D(d);
	return cycleToSecond(d->deltaCycles);
}

// platforms/[os]/timer.cpp
extern "C" GMLargeInteger highResolutionTimerFrequency();
extern "C" GMLargeInteger highResolutionTimer();

GMLargeInteger GMClock::highResolutionTimerFrequency()
{
	return ::highResolutionTimerFrequency();
}

GMLargeInteger GMClock::highResolutionTimer()
{
	return ::highResolutionTimer();
}

GMfloat GMClock::cycleToSecond(GMLargeInteger cycle)
{
	D(d);
	return cycle / (GMfloat)d->frequency;
}

//GMStopwatch
GMStopwatch::GMStopwatch()
{
	D(d);
	d->frequency = GMClock::highResolutionTimerFrequency();
	d->start = 0;
	d->end = 0;
}

void GMStopwatch::start()
{
	D(d);
	d->start = GMClock::highResolutionTimer();
}

void GMStopwatch::stop()
{
	D(d);
	d->end = GMClock::highResolutionTimer();
}

GMfloat GMStopwatch::timeInSecond()
{
	D(d);
	return timeInCycle() / (GMfloat)d->frequency;
}

GMLargeInteger GMStopwatch::timeInCycle()
{
	D(d);
	return d->end - d->start;
}

GMfloat GMStopwatch::nowInSecond()
{
	D(d);
	return nowInCycle() / (GMfloat)d->frequency;
}

GMLargeInteger GMStopwatch::nowInCycle()
{
	D(d);
	auto now = GMClock::highResolutionTimer();
	return now - d->start;
}

//Plane
#define EPSILON 0.01f

void GMPlane::setFromPoints(const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3 & p2)
{
	normal = glm::cross((p1 - p0), (p2 - p0));
	normal = glm::fastNormalize(normal);
	calculateIntercept(p0);
}

void GMPlane::normalize()
{
	GMfloat normalLength = normal.length();
	normal /= normalLength;
	intercept /= normalLength;
}

bool GMPlane::intersect3(const GMPlane & p2, const GMPlane & p3, glm::vec3 & result)//find point of intersection of 3 planes
{
	GMfloat denominator = glm::dot(normal, (glm::cross(p2.normal, p3.normal)));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	glm::vec3 temp1, temp2, temp3;
	temp1 = (glm::cross(p2.normal, p3.normal))*intercept;
	temp2 = (glm::cross(p3.normal, normal))*p2.intercept;
	temp3 = (glm::cross(normal, p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat GMPlane::getDistance(const glm::vec3 & point) const
{
	return glm::dot(point, normal) + intercept;
}

PointPosition GMPlane::classifyPoint(const glm::vec3 & point) const
{
	GMfloat distance = getDistance(point);

	if (distance > EPSILON)	//==0.0f is too exact, give a bit of room
		return POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return POINT_BEHIND_PLANE;

	return POINT_ON_PLANE;	//otherwise
}

GMPlane GMPlane::lerp(const GMPlane & p2, GMfloat factor)
{
	GMPlane result;
	result.normal = normal*(1.0f - factor) + p2.normal*factor;
	result.normal = glm::fastNormalize(result.normal);

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool GMPlane::operator ==(const GMPlane & rhs) const
{
	if ((normal == rhs.normal) && (intercept == rhs.intercept))
		return true;

	return false;
}

//Frustum
enum FRUSTUM_PLANES
{
	LEFT_PLANE = 0,
	RIGHT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE
};

void GMFrustum::initOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Orthographic;
	d->left = left;
	d->right = right;
	d->bottom = bottom;
	d->top = top;
	d->n = n;
	d->f = f;
}

void GMFrustum::initPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Perspective;
	d->fovy = fovy;
	d->aspect = aspect;
	d->n = n;
	d->f = f;
}

void GMFrustum::update()
{
	D(d);
	glm::mat4 projection = getProjection();
	glm::mat4& view = d->viewMatrix;
	glm::mat4 clipMat;

	if (d->type == GMFrustumType::Perspective)
	{
		//Multiply the matrices
		clipMat = projection * view;

		GMfloat clip[16];
		for (GMint i = 0; i < 4; i++)
		{
			glm::vec4 vec = clipMat[i];
			for (GMint j = 0; j < 4; j++)
			{
				clip[i * 4 + j] = vec[j];
			}
		}

		//calculate planes
		d->planes[RIGHT_PLANE].normal = glm::vec3(clip[3] - clip[0], clip[7] - clip[4], clip[11] - clip[8]);
		d->planes[RIGHT_PLANE].intercept = clip[15] - clip[12];

		d->planes[LEFT_PLANE].normal = glm::vec3(clip[3] + clip[0], clip[7] + clip[4], clip[11] + clip[8]);
		d->planes[LEFT_PLANE].intercept = clip[15] + clip[12];

		d->planes[BOTTOM_PLANE].normal = glm::vec3(clip[3] + clip[1], clip[7] + clip[5], clip[11] + clip[9]);
		d->planes[BOTTOM_PLANE].intercept = clip[15] + clip[13];

		d->planes[TOP_PLANE].normal = glm::vec3(clip[3] - clip[1], clip[7] - clip[5], clip[11] - clip[9]);
		d->planes[TOP_PLANE].intercept = clip[15] - clip[13];

		d->planes[FAR_PLANE].normal = glm::vec3(clip[3] - clip[2], clip[7] - clip[6], clip[11] - clip[10]);
		d->planes[FAR_PLANE].intercept = clip[15] - clip[14];

		d->planes[NEAR_PLANE].normal = glm::vec3(clip[3] + clip[2], clip[7] + clip[6], clip[11] + clip[10]);
		d->planes[NEAR_PLANE].intercept = clip[15] + clip[14];
	}
	else
	{
		GM_ASSERT(d->type == GMFrustumType::Orthographic);
		d->planes[RIGHT_PLANE].normal = glm::vec3(1, 0, 0);
		d->planes[RIGHT_PLANE].intercept = d->right;

		d->planes[LEFT_PLANE].normal = glm::vec3(-1, 0, 0);
		d->planes[LEFT_PLANE].intercept = d->left;

		d->planes[BOTTOM_PLANE].normal = glm::vec3(0, -1, 0);
		d->planes[BOTTOM_PLANE].intercept = d->bottom;

		d->planes[TOP_PLANE].normal = glm::vec3(0, 1, 0);
		d->planes[TOP_PLANE].intercept = d->top;

		d->planes[NEAR_PLANE].normal = glm::vec3(0, 0, 1);
		d->planes[NEAR_PLANE].intercept = d->n;

		d->planes[FAR_PLANE].normal = glm::vec3(0, 0, -1);
		d->planes[FAR_PLANE].intercept = d->f;
	}

	//normalize planes
	for (int i = 0; i < 6; ++i)
		d->planes[i].normalize();
}

//is a point in the Frustum?
bool GMFrustum::isPointInside(const glm::vec3 & point)
{
	D(d);
	for (int i = 0; i < 6; ++i)
	{
		if (d->planes[i].classifyPoint(point) == POINT_BEHIND_PLANE)
			return false;
	}

	return true;
}

//is a bounding box in the Frustum?
bool GMFrustum::isBoundingBoxInside(const glm::vec3 * vertices)
{
	D(d);
	for (int i = 0; i < 6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if (d->planes[i].classifyPoint(vertices[0]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[1]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[2]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[3]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[4]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[5]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[6]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[7]) != POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;
	}

	return true;
}

glm::mat4 GMFrustum::getProjection()
{
	D(d);
	if (d->type == GMFrustumType::Perspective)
		return glm::perspective(d->fovy, d->aspect, d->n, d->f);
	return glm::ortho(d->left, d->right, d->bottom, d->top, d->n, d->f);
}

void GMFrustum::updateViewMatrix(glm::mat4& viewMatrix, glm::mat4& projMatrix)
{
	D(d);
	d->viewMatrix = viewMatrix;
	d->projMatrix = projMatrix;
}

//Scanner
static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

Scanner::Scanner(const char* line)
{
	D(d);
	d->p = line;
	d->predicate = isWhiteSpace;
	d->skipSame = true;
	d->valid = !!d->p;
}

Scanner::Scanner(const char* line, CharPredicate predicate)
{
	D(d);
	d->p = line;
	d->predicate = predicate;
	d->skipSame = true;
	d->valid = !!d->p;
}

Scanner::Scanner(const char* line, bool skipSame, CharPredicate predicate)
{
	D(d);
	d->p = line;
	d->predicate = predicate;
	d->skipSame = skipSame;
	d->valid = !!d->p;
}

void Scanner::next(char* out)
{
	D(d);
	if (!d->valid)
	{
		strcpy_s(out, 1, "");
		return;
	}

	char* p = out;
	bool b = false;
	if (!d->p)
	{
		strcpy_s(out, 1, "");
		return;
	}

	while (*d->p && d->predicate(*d->p))
	{
		if (b && !d->skipSame)
		{
			*p = 0;
			d->p++;
			return;
		}
		d->p++;
		b = true;
	}

	if (!*d->p)
	{
		*p = 0;
		return;
	}

	do
	{
		*p = *d->p;
		p++;
		d->p++;
	} while (*d->p && !d->predicate(*d->p));

	*p = 0;
}

void Scanner::nextToTheEnd(char* out)
{
	D(d);
	if (!d->valid)
		return;

	char* p = out;
	while (*d->p)
	{
		d->p++;
		*p = *d->p;
		p++;
	}
}

bool Scanner::nextFloat(GMfloat* out)
{
	D(d);
	if (!d->valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	SAFE_SSCANF(command, "%f", out);
	return true;
}

bool Scanner::nextInt(GMint* out)
{
	D(d);
	if (!d->valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	SAFE_SSCANF(command, "%i", out);
	return true;
}

//MemoryStream

GMMemoryStream::GMMemoryStream(const GMbyte* buffer, GMuint size)
{
	D(d);
	d->start = buffer;
	d->size = size;
	d->ptr = buffer;
	d->end = d->start + d->size;
}

GMuint GMMemoryStream::read(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	d->ptr += realSize;
	return realSize;
}

GMuint GMMemoryStream::peek(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	return realSize;
}

void GMMemoryStream::rewind()
{
	D(d);
	d->ptr = d->start;
}

GMuint GMMemoryStream::size()
{
	D(d);
	return d->size;
}

GMuint GMMemoryStream::tell()
{
	D(d);
	return d->ptr - d->start;
}

GMbyte GMMemoryStream::get()
{
	GMbyte c;
	read(&c, 1);
	return c;
}

void GMMemoryStream::seek(GMuint cnt, SeekMode mode)
{
	D(d);
	if (mode == GMMemoryStream::FromStart)
		d->ptr = d->start + cnt;
	else if (mode == GMMemoryStream::FromNow)
		d->ptr += cnt;
	else
		GM_ASSERT(false);
}

//Bitset
bool Bitset::init(GMint numberOfBits)
{
	D(d);
	//Delete any memory allocated to bits
	GM_delete_array(d->bits);

	//Calculate size
	d->numBytes = (numberOfBits >> 3) + 1;

	//Create memory
	d->bits = new unsigned char[d->numBytes];
	if (!d->bits)
	{
		gm_error(_L("Unable to allocate space for a Bitset of %d bits"), numberOfBits);
		return false;
	}

	clearAll();

	return true;
}