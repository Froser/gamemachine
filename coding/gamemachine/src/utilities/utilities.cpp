#include "stdafx.h"
#include "utilities.h"
#include <time.h>
#include "vmath.h"
#include "assert.h"
#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
#endif
#include <vector>

//FPSCounter
FPSCounter::FPSCounter()
{
	D(d);
	d.fps = 0;
	d.lastTime = 0.f;
	d.frames = 0.f;
	d.time = 0.f;
	d.immediate_lastTime = 0.f;
	d.elapsed_since_last_frame = 0.f;
}

// 每一帧运行一次update
void FPSCounter::update()
{
	D(d);
	d.time = clock() * 0.001f;								//get current time in seconds
	++d.frames;												//increase frame count

	if (d.time - d.lastTime > 1.0f)							//if it has been 1 second
	{
		d.fps = d.frames / (d.time - d.lastTime);			//update fps number
		d.lastTime = d.time;								//set beginning count
		d.frames = 0L;										//reset frames this second
	}

	d.elapsed_since_last_frame = d.time - d.immediate_lastTime;
	d.immediate_lastTime = d.time;
}

GMfloat FPSCounter::getFps()
{
	D(d);
	return d.fps;
}

GMfloat FPSCounter::getElapsedSinceLastFrame()
{
	D(d);
	return d.elapsed_since_last_frame;
}

//Plane
#define EPSILON 0.01f

void Plane::setFromPoints(const vmath::vec3 & p0, const vmath::vec3 & p1, const vmath::vec3 & p2)
{

	normal = vmath::cross((p1 - p0), (p2 - p0));

	normal = vmath::normalize(normal);

	calculateIntercept(p0);
}

void Plane::normalize()
{
	float normalLength = vmath::length(normal);
	normal /= normalLength;
	intercept /= normalLength;
}

bool Plane::intersect3(const Plane & p2, const Plane & p3, vmath::vec3 & result)//find point of intersection of 3 planes
{
	float denominator = vmath::dot(normal, (vmath::cross(p2.normal, p3.normal)));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	vmath::vec3 temp1, temp2, temp3;
	temp1 = (vmath::cross(p2.normal, p3.normal))*intercept;
	temp2 = (vmath::cross(p3.normal, normal))*p2.intercept;
	temp3 = (vmath::cross(normal, p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat Plane::getDistance(const vmath::vec3 & point) const
{
	return point[0] * normal[0] + point[1] * normal[1] + point[2] * normal[2] + intercept;
}

PointPosition Plane::classifyPoint(const vmath::vec3 & point) const
{
	float distance = point[0] * normal[0] + point[1] * normal[1] + point[2] * normal[2] + intercept;

	if (distance > EPSILON)	//==0.0f is too exact, give a bit of room
		return POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return POINT_BEHIND_PLANE;

	return POINT_ON_PLANE;	//otherwise
}

Plane Plane::lerp(const Plane & p2, float factor)
{
	Plane result;
	result.normal = normal*(1.0f - factor) + p2.normal*factor;
	result.normal = vmath::normalize(result.normal);

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool Plane::operator ==(const Plane & rhs) const
{
	if (normal == rhs.normal && intercept == rhs.intercept)
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

Frustum::Frustum(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
	: m_fovy(fovy)
	, m_aspect(aspect)
	, m_n(n)
	, m_f(f)
{

}

void Frustum::update()
{
	vmath::mat4 projection = getPerspective();
	vmath::mat4& view = m_viewMatrix;
	vmath::mat4 clipMat;

	//Multiply the matrices
	clipMat = projection * view;

	GMfloat clip[16];
	for (GMint i = 0; i < 4; i++)
	{
		vmath::vec4 vec = clipMat[i];
		for (GMint j = 0; j < 4; j++)
		{
			clip[i * 4 + j] = vec[j];
		}
	}

	//calculate planes
	planes[RIGHT_PLANE].normal[0] = clip[3] - clip[0];
	planes[RIGHT_PLANE].normal[1] = clip[7] - clip[4];
	planes[RIGHT_PLANE].normal[2] = clip[11] - clip[8];
	planes[RIGHT_PLANE].intercept = clip[15] - clip[12];

	planes[LEFT_PLANE].normal[0] = clip[3] + clip[0];
	planes[LEFT_PLANE].normal[1] = clip[7] + clip[4];
	planes[LEFT_PLANE].normal[2] = clip[11] + clip[8];
	planes[LEFT_PLANE].intercept = clip[15] + clip[12];

	planes[BOTTOM_PLANE].normal[0] = clip[3] + clip[1];
	planes[BOTTOM_PLANE].normal[1] = clip[7] + clip[5];
	planes[BOTTOM_PLANE].normal[2] = clip[11] + clip[9];
	planes[BOTTOM_PLANE].intercept = clip[15] + clip[13];

	planes[TOP_PLANE].normal[0] = clip[3] - clip[1];
	planes[TOP_PLANE].normal[1] = clip[7] - clip[5];
	planes[TOP_PLANE].normal[2] = clip[11] - clip[9];
	planes[TOP_PLANE].intercept = clip[15] - clip[13];

	planes[FAR_PLANE].normal[0] = clip[3] - clip[2];
	planes[FAR_PLANE].normal[1] = clip[7] - clip[6];
	planes[FAR_PLANE].normal[2] = clip[11] - clip[10];
	planes[FAR_PLANE].intercept = clip[15] - clip[14];

	planes[NEAR_PLANE].normal[0] = clip[3] + clip[2];
	planes[NEAR_PLANE].normal[1] = clip[7] + clip[6];
	planes[NEAR_PLANE].normal[2] = clip[11] + clip[10];
	planes[NEAR_PLANE].intercept = clip[15] + clip[14];

	//normalize planes
	for (int i = 0; i < 6; ++i)
		planes[i].normalize();
}

//is a point in the Frustum?
bool Frustum::isPointInside(const vmath::vec3 & point)
{
	for (int i = 0; i < 6; ++i)
	{
		if (planes[i].classifyPoint(point) == POINT_BEHIND_PLANE)
			return false;
	}

	return true;
}

//is a bounding box in the Frustum?
bool Frustum::isBoundingBoxInside(const vmath::vec3 * vertices)
{
	//loop through planes
	for (int i = 0; i < 6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if (planes[i].classifyPoint(vertices[0]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[1]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[2]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[3]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[4]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[5]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[6]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[7]) != POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;
	}

	return true;
}

vmath::mat4 Frustum::getPerspective()
{
	return vmath::mat4(vmath::perspective(m_fovy, m_aspect, m_n, m_f));;
}

void Frustum::updateViewMatrix(vmath::mat4& viewMatrix, vmath::mat4& projMatrix)
{
	m_viewMatrix = viewMatrix;
	m_projMatrix = projMatrix;
}

//Scanner
static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

Scanner::Scanner(const char* line)
	: m_p(line)
	, m_predicate(isWhiteSpace)
	, m_skipSame(true)
{
	m_valid = !!m_p;
}

Scanner::Scanner(const char* line, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(true)
{
	m_valid = !!m_p;
}

Scanner::Scanner(const char* line, bool skipSame, CharPredicate predicate)
	: m_p(line)
	, m_predicate(predicate)
	, m_skipSame(skipSame)
{
	m_valid = !!m_p;
}

void Scanner::next(char* out)
{
	if (!m_valid)
	{
		strcpy_s(out, 1, "");
		return;
	}

	char* p = out;
	bool b = false;
	if (!m_p)
	{
		strcpy_s(out, 1, "");
		return;
	}

	while (*m_p && m_predicate(*m_p))
	{
		if (b && !m_skipSame)
		{
			*p = 0;
			m_p++;
			return;
		}
		m_p++;
		b = true;
	}

	if (!*m_p)
	{
		*p = 0;
		return;
	}

	do
	{
		*p = *m_p;
		p++;
		m_p++;
	} while (*m_p && !m_predicate(*m_p));

	*p = 0;
}

void Scanner::nextToTheEnd(char* out)
{
	if (!m_valid)
		return;

	char* p = out;
	while (*m_p)
	{
		m_p++;
		*p = *m_p;
		p++;
	}
}

bool Scanner::nextFloat(GMfloat* out)
{
	if (!m_valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%f", out);
	return true;
}

bool Scanner::nextInt(GMint* out)
{
	if (!m_valid)
		return false;

	char command[LINE_MAX];
	next(command);
	if (!strlen(command))
		return false;
	sscanf_s(command, "%i", out);
	return true;
}

//MemoryStream

MemoryStream::MemoryStream(const GMbyte* data, GMuint size)
	: m_start(data)
	, m_size(size)
	, m_ptr(data)
{
	m_end = m_start + m_size;
}

GMuint MemoryStream::read(GMbyte* buf, GMuint size)
{
	if (m_ptr >= m_end)
		return 0;

	GMuint realSize = m_ptr + size > m_end ? m_end - m_ptr : size;
	memcpy(buf, m_ptr, realSize);
	m_ptr += realSize;
	return realSize;
}

GMuint MemoryStream::peek(GMbyte* buf, GMuint size)
{
	if (m_ptr >= m_end)
		return 0;

	GMuint realSize = m_ptr + size > m_end ? m_end - m_ptr : size;
	memcpy(buf, m_ptr, realSize);
	return realSize;
}

void MemoryStream::rewind()
{
	m_ptr = m_start;
}

GMuint MemoryStream::size()
{
	return m_size;
}

GMuint MemoryStream::tell()
{
	return m_ptr - m_start;
}

GMbyte MemoryStream::get()
{
	GMbyte c;
	read(&c, 1);
	return c;
}

void MemoryStream::seek(GMuint cnt, SeekMode mode)
{
	if (mode == MemoryStream::FromStart)
		m_ptr = m_start + cnt;
	else if (mode == MemoryStream::FromNow)
		m_ptr += cnt;
	else
		ASSERT(false);
}

//Bitset

bool Bitset::init(int numberOfBits)
{
	//Delete any memory allocated to bits
	if (bits)
		delete[] bits;
	bits = NULL;

	//Calculate size
	numBytes = (numberOfBits >> 3) + 1;

	//Create memory
	bits = new unsigned char[numBytes];
	if (!bits)
	{
		gm_error("Unable to allocate space for a Bitset of %d bits", numberOfBits);
		return false;
	}

	clearAll();

	return true;
}

void Bitset::clearAll()
{
	memset(bits, 0, numBytes);
}

void Bitset::setAll()
{
	memset(bits, 0xFF, numBytes);
}

void Bitset::clear(int bitNumber)
{
	bits[bitNumber >> 3] &= ~(1 << (bitNumber & 7));
}

void Bitset::set(int bitNumber)
{
	bits[bitNumber >> 3] |= 1 << (bitNumber & 7);
}

unsigned char Bitset::isSet(int bitNumber)
{
	return bits[bitNumber >> 3] & 1 << (bitNumber & 7);
}

//Camera
void Camera::calcCameraLookAt(const PositionState& state, REF CameraLookAt& lookAt)
{
	lookAt.lookAt[1] = std::sin(state.pitch);
	GMfloat l = std::cos(state.pitch);
	lookAt.lookAt[0] = l * std::sin(state.yaw);
	lookAt.lookAt[2] = -l * std::cos(state.yaw);

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

std::vector<std::string> Path::getAllFiles(const char* directory)
{
	std::vector<std::string> res;
#if _WINDOWS
	std::string p = directory;
	p.append("*");
	_finddata_t d;
	long hFile = 0;
	if ((hFile = _findfirst(p.c_str(), &d)) != -1)
	{
		do
		{
			if ((d.attrib &  _A_ARCH))
			{
				if (!strEqual(d.name, ".") && !strEqual(d.name, ".."))
					res.push_back(std::string(directory).append(d.name));
			}
		} while (_findnext(hFile, &d) == 0);
		_findclose(hFile);
	}
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
#else
#error need implement
#endif
}