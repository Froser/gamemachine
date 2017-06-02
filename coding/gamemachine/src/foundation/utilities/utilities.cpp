#include "stdafx.h"
#include "utilities.h"
#include "foundation/linearmath.h"
#include "assert.h"
#ifdef _WINDOWS
#	include <io.h>
#	include <direct.h>
#endif
#include "foundation/vector.h"

//FPSCounter
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

GMLargeInteger GMClock::highResolutionTimerFrequency()
{
#ifdef _WINDOWS
	LARGE_INTEGER i;
	BOOL b = QueryPerformanceFrequency(&i);
	ASSERT(b);
	return i.QuadPart;
#else
	ASSERT(false);
	return 0;
#endif
}

GMLargeInteger GMClock::highResolutionTimer()
{
#ifdef _WINDOWS
	LARGE_INTEGER i;
	BOOL b = QueryPerformanceCounter(&i);
	ASSERT(b);
	return i.QuadPart;
#else
	ASSERT(false);
	return 0;
#endif
}

GMfloat GMClock::cycleToSecond(GMLargeInteger cycle)
{
	D(d);
	return cycle / (GMfloat)d->frequency;
}

//Plane
#define EPSILON 0.01f

void Plane::setFromPoints(const linear_math::Vector3 & p0, const linear_math::Vector3 & p1, const linear_math::Vector3 & p2)
{

	normal = linear_math::cross((p1 - p0), (p2 - p0));

	normal = linear_math::normalize(normal);

	calculateIntercept(p0);
}

void Plane::normalize()
{
	GMfloat normalLength = linear_math::length(normal);
	normal /= normalLength;
	intercept /= normalLength;
}

bool Plane::intersect3(const Plane & p2, const Plane & p3, linear_math::Vector3 & result)//find point of intersection of 3 planes
{
	GMfloat denominator = linear_math::dot(normal, (linear_math::cross(p2.normal, p3.normal)));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	linear_math::Vector3 temp1, temp2, temp3;
	temp1 = (linear_math::cross(p2.normal, p3.normal))*intercept;
	temp2 = (linear_math::cross(p3.normal, normal))*p2.intercept;
	temp3 = (linear_math::cross(normal, p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat Plane::getDistance(const linear_math::Vector3 & point) const
{
	return linear_math::dot(point, normal) + intercept;
}

PointPosition Plane::classifyPoint(const linear_math::Vector3 & point) const
{
	GMfloat distance = getDistance(point);

	if (distance > EPSILON)	//==0.0f is too exact, give a bit of room
		return POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return POINT_BEHIND_PLANE;

	return POINT_ON_PLANE;	//otherwise
}

Plane Plane::lerp(const Plane & p2, GMfloat factor)
{
	Plane result;
	result.normal = normal*(1.0f - factor) + p2.normal*factor;
	result.normal = linear_math::normalize(result.normal);

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool Plane::operator ==(const Plane & rhs) const
{
	if (equals(normal, rhs.normal) && intercept == rhs.intercept)
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

void Frustum::initFrustum(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->fovy = fovy;
	d->aspect = aspect;
	d->n = n;
	d->f = f;
}

void Frustum::update()
{
	D(d);
	linear_math::Matrix4x4 projection = getPerspective();
	linear_math::Matrix4x4& view = d->viewMatrix;
	linear_math::Matrix4x4 clipMat;

	//Multiply the matrices
	clipMat = projection * view;

	GMfloat clip[16];
	for (GMint i = 0; i < 4; i++)
	{
		linear_math::Vector4 vec = clipMat[i];
		for (GMint j = 0; j < 4; j++)
		{
			clip[i * 4 + j] = vec[j];
		}
	}

	//calculate planes
	d->planes[RIGHT_PLANE].normal[0] = clip[3] - clip[0];
	d->planes[RIGHT_PLANE].normal[1] = clip[7] - clip[4];
	d->planes[RIGHT_PLANE].normal[2] = clip[11] - clip[8];
	d->planes[RIGHT_PLANE].intercept = clip[15] - clip[12];

	d->planes[LEFT_PLANE].normal[0] = clip[3] + clip[0];
	d->planes[LEFT_PLANE].normal[1] = clip[7] + clip[4];
	d->planes[LEFT_PLANE].normal[2] = clip[11] + clip[8];
	d->planes[LEFT_PLANE].intercept = clip[15] + clip[12];

	d->planes[BOTTOM_PLANE].normal[0] = clip[3] + clip[1];
	d->planes[BOTTOM_PLANE].normal[1] = clip[7] + clip[5];
	d->planes[BOTTOM_PLANE].normal[2] = clip[11] + clip[9];
	d->planes[BOTTOM_PLANE].intercept = clip[15] + clip[13];

	d->planes[TOP_PLANE].normal[0] = clip[3] - clip[1];
	d->planes[TOP_PLANE].normal[1] = clip[7] - clip[5];
	d->planes[TOP_PLANE].normal[2] = clip[11] - clip[9];
	d->planes[TOP_PLANE].intercept = clip[15] - clip[13];

	d->planes[FAR_PLANE].normal[0] = clip[3] - clip[2];
	d->planes[FAR_PLANE].normal[1] = clip[7] - clip[6];
	d->planes[FAR_PLANE].normal[2] = clip[11] - clip[10];
	d->planes[FAR_PLANE].intercept = clip[15] - clip[14];

	d->planes[NEAR_PLANE].normal[0] = clip[3] + clip[2];
	d->planes[NEAR_PLANE].normal[1] = clip[7] + clip[6];
	d->planes[NEAR_PLANE].normal[2] = clip[11] + clip[10];
	d->planes[NEAR_PLANE].intercept = clip[15] + clip[14];

	//normalize planes
	for (int i = 0; i < 6; ++i)
		d->planes[i].normalize();
}

//is a point in the Frustum?
bool Frustum::isPointInside(const linear_math::Vector3 & point)
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
bool Frustum::isBoundingBoxInside(const linear_math::Vector3 * vertices)
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

linear_math::Matrix4x4 Frustum::getPerspective()
{
	D(d);
	return linear_math::Matrix4x4(linear_math::perspective(d->fovy, d->aspect, d->n, d->f));;
}

void Frustum::updateViewMatrix(linear_math::Matrix4x4& viewMatrix, linear_math::Matrix4x4& projMatrix)
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

MemoryStream::MemoryStream(const GMbyte* buffer, GMuint size)
{
	D(d);
	d->start = buffer;
	d->size = size;
	d->ptr = buffer;
	d->end = d->start + d->size;
}

GMuint MemoryStream::read(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	d->ptr += realSize;
	return realSize;
}

GMuint MemoryStream::peek(GMbyte* buf, GMuint size)
{
	D(d);
	if (d->ptr >= d->end)
		return 0;

	GMuint realSize = d->ptr + size > d->end ? d->end - d->ptr : size;
	memcpy(buf, d->ptr, realSize);
	return realSize;
}

void MemoryStream::rewind()
{
	D(d);
	d->ptr = d->start;
}

GMuint MemoryStream::size()
{
	D(d);
	return d->size;
}

GMuint MemoryStream::tell()
{
	D(d);
	return d->ptr - d->start;
}

GMbyte MemoryStream::get()
{
	GMbyte c;
	read(&c, 1);
	return c;
}

void MemoryStream::seek(GMuint cnt, SeekMode mode)
{
	D(d);
	if (mode == MemoryStream::FromStart)
		d->ptr = d->start + cnt;
	else if (mode == MemoryStream::FromNow)
		d->ptr += cnt;
	else
		ASSERT(false);
}

//Bitset
bool Bitset::init(GMint numberOfBits)
{
	D(d);
	//Delete any memory allocated to bits
	if (d->bits)
		delete[] d->bits;
	d->bits = NULL;

	//Calculate size
	d->numBytes = (numberOfBits >> 3) + 1;

	//Create memory
	d->bits = new unsigned char[d->numBytes];
	if (!d->bits)
	{
		gm_error("Unable to allocate space for a Bitset of %d bits", numberOfBits);
		return false;
	}

	clearAll();

	return true;
}

void Bitset::clearAll()
{
	D(d);
	memset(d->bits, 0, d->numBytes);
}

void Bitset::setAll()
{
	D(d);
	memset(d->bits, 0xFF, d->numBytes);
}

void Bitset::clear(GMint bitNumber)
{
	D(d);
	d->bits[bitNumber >> 3] &= ~(1 << (bitNumber & 7));
}

void Bitset::set(GMint bitNumber)
{
	D(d);
	d->bits[bitNumber >> 3] |= 1 << (bitNumber & 7);
}

GMbyte Bitset::isSet(GMint bitNumber)
{
	D(d);
	return d->bits[bitNumber >> 3] & 1 << (bitNumber & 7);
}

//Camera
void Camera::calcCameraLookAt(const PositionState& state, REF CameraLookAt& lookAt)
{
	lookAt.lookAt[1] = sin(state.pitch);
	GMfloat l = cos(state.pitch);
	lookAt.lookAt[0] = l * sin(state.yaw);
	lookAt.lookAt[2] = -l * cos(state.yaw);

	lookAt.position = state.position;
}

//Path

std::string Path::directoryName(const std::string& fileName)
{
	int pos1 = fileName.find_last_of('\\'),
		pos2 = fileName.find_last_of('/');
	int pos = pos1 > pos2 ? pos1 : pos2;
	if (pos == std::string::npos)
		return fileName;
	return fileName.substr(0, pos + 1);
}

std::string Path::filename(const std::string& fullPath)
{
	int pos1 = fullPath.find_last_of('\\'),
		pos2 = fullPath.find_last_of('/');
	int pos = pos1 > pos2 ? pos1 : pos2;
	if (pos == std::string::npos)
		return fullPath;
	return fullPath.substr(pos + 1, fullPath.size());
}

std::string Path::getCurrentPath()
{
#if _WINDOWS
	const int MAX = 255;
	CHAR fn[MAX];
	::GetModuleFileName(NULL, fn, MAX);
	return directoryName(fn);
#endif
	return "";
}

AlignedVector<std::string> Path::getAllFiles(const char* directory)
{
	AlignedVector<std::string> res;
#if _WINDOWS
	std::string p = directory;
	p.append("*");
	_finddata_t fd;
	long hFile = 0;
	if ((hFile = _findfirst(p.c_str(), &fd)) != -1)
	{
		do
		{
			if ((fd.attrib &  _A_ARCH))
			{
				if (!strEqual(fd.name, ".") && !strEqual(fd.name, ".."))
					res.push_back(std::string(directory).append(fd.name));
			}
		} while (_findnext(hFile, &fd) == 0);
		_findclose(hFile);
	}
#elif defined __APPLE__
	ASSERT(false);
	return AlignedVector<std::string>();
#else
#error need implement
#endif
	return res;
}

bool Path::directoryExists(const std::string& dir)
{
#ifdef _WINDOWS
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(dir.c_str(), &findFileData);

	bool b = hFind != INVALID_HANDLE_VALUE;
	FindClose(hFind);
	return b;
#elif defined __APPLE__
	ASSERT(false);
	return false;
#else
#error need implement
#endif
}

void Path::createDirectory(const std::string& dir)
{
#ifdef _WINDOWS
	if (directoryExists(dir) || (dir.size() == 2 && dir[1] == ':'))
		return;

	std::string up = dir;
	if (up.back() == '/' || up.back() == '\\')
		up = up.substr(0, up.size() - 1); //去掉斜杠和反斜杠
	up = directoryName(up);
	for (GMuint i = 0; i < up.size(); i++)
	{
		if (up[i] == '/')
			up[i] = '\\';
	}
	if (up.back() == '/' || up.back() == '\\')
		up = up.substr(0, up.size() - 1);

	createDirectory(up);
	_mkdir(dir.c_str());
#elif defined __APPLE__
	ASSERT(false);
#else
#error need implement
#endif
}
