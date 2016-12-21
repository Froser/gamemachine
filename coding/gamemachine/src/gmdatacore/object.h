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
};

class Component
{
	friend Object;

public:
	Component();
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

private:
	GMuint m_count;
	GMuint m_offset;
	Material m_material;
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

	void appendComponent(Component* component, GMuint count)
	{
		component->m_count = count;
		m_components.push_back(component);
	}

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

	ArrayData<GMuint>& ebo()
	{
		return m_indices;
	}

	void setIndices(ArrayData<GMuint>& indices)
	{
		m_indices = indices;
	}

	void setNormals(ArrayData<GMfloat>& normals)
	{
		m_normals = normals;
	}

	ArrayData<GMfloat>& normals()
	{
		return m_normals;
	}

	GMuint getElementBufferId() { return m_elementBufferId; }
	void setElementBufferId(GMuint id) { m_elementBufferId = id; }
	GMuint getBufferId() { return m_bufferId; }
	GMuint getArrayId() { return m_arrayId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }

private:
	ArrayData<GMfloat> m_vertices;
	ArrayData<GMfloat> m_normals;
	ArrayData<GMuint> m_indices;
	GMuint m_arrayId;
	GMuint m_bufferId;
	GMuint m_elementBufferId;
	ObjectDrawer* m_drawer;
	std::vector<Component*> m_components;
};

END_NS
#endif