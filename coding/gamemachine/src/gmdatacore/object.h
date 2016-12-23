#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <vector>
#include "image.h"

BEGIN_NS

class Object;
class GMGLShaders;
struct ObjectDrawer
{
	virtual void init(Object*) = 0;
	virtual void draw(GMGLShaders&, Object*) = 0;
	virtual void dispose(Object*) = 0;
};

template <typename T>
struct ArrayData
{
	T* data;
	GMuint size;
};

struct Material
{
	GMfloat Ka[3];
	GMfloat Kd[3];
	GMfloat Ks[3];
	GMfloat shininess;
};

class Component
{
	friend Object;

public:
	Component(GMuint edgeCountPerPolygon);
	~Component();

	Material& getMaterial()
	{
		return m_material;
	}

	void setOffset(GMuint offset)
	{
		m_offset = offset;
	}

	GMuint getOffset()
	{
		return m_offset;
	}

	GMuint getCount()
	{
		return m_count;
	}

	GMint* getFirstPtr()
	{
		return m_firstPtr;
	}

	GMint* getCountPtr()
	{
		return m_countPtr;
	}

	GMuint getPolygonCount()
	{
		return m_count / m_edgeCountPerPolygon;
	}

	void generatePolygonProperties();

private:
	GMuint m_count;
	GMuint m_offset;
	Material m_material;
	GMuint m_edgeCountPerPolygon;

// 多边形属性
	GMint* m_firstPtr;
	GMint* m_countPtr;
};

class Object
{
public:
	Object();
	~Object();

	ObjectDrawer* getDrawer()
	{
		return m_drawer;
	}

	void appendComponent(Component* component, GMuint count);

	std::vector<Component*>& getComponents()
	{
		return m_components;
	}

	void setVertices(ArrayData<GMfloat>& vertices)
	{
		m_vertices = vertices;
	}

	ArrayData<GMfloat>& vao()
	{
		return m_vertices;
	}

	void setNormals(ArrayData<GMfloat>& normals)
	{
		m_normals = normals;
	}

	ArrayData<GMfloat>& normals()
	{
		return m_normals;
	}

	GMuint getBufferId() { return m_bufferId; }
	GMuint getArrayId() { return m_arrayId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }

private:
	ArrayData<GMfloat> m_vertices;
	ArrayData<GMfloat> m_normals;
	GMuint m_arrayId;
	GMuint m_bufferId;
	ObjectDrawer* m_drawer;
	std::vector<Component*> m_components;
};

END_NS
#endif