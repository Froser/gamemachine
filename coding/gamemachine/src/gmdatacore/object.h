#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <map>
#include "image.h"

BEGIN_NS

class Object;
struct ObjectDrawer
{
	virtual void init(Object*) = 0;
	virtual void draw(Object*) = 0;
	virtual void dispose(Object*) = 0;
};

template <typename T>
struct ArrayData
{
	T* data;
	GMuint size;
};

class Object
{
public:
	Object();
	~Object();

public:
	void setVertices(ArrayData<GMfloat> vertices)
	{
		m_vertices = vertices;
	}

	ArrayData<GMfloat>& vao()
	{
		return m_vertices;
	}

	ArrayData<GMuint>& ebo()
	{
		return m_indices;
	}

	void setIndices(ArrayData<GMuint> indices)
	{
		m_indices = indices;
	}

	ObjectDrawer* getDrawer()
	{
		return m_drawer;
	}

	GMuint getArrayId() { return m_arrayId; }
	GMuint getBufferId() { return m_bufferId; }
	GMuint getElementBufferId() { return m_elementBufferId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }
	void setElementBufferId(GMuint id) { m_elementBufferId = id; }

private:
	ArrayData<GMfloat> m_vertices;
	ArrayData<GMuint> m_indices;
	GMuint m_arrayId;
	GMuint m_bufferId;
	GMuint m_elementBufferId;
	ObjectDrawer* m_drawer;
};

END_NS
#endif