#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <vector>
#include "image.h"
#include "utilities/assert.h"
#include "utilities/autoptr.h"
#include "utilities/vmath.h"
#include "gmdatacore/texture.h"

BEGIN_NS

class Object;
class GMGLShaders;
class ObjectPainter
{
public:
	ObjectPainter(Object* obj);

public:
	virtual void init() = 0;
	virtual void draw() = 0;
	virtual void dispose() = 0;

protected:
	Object* getObject();

private:
	Object* m_object;
};

struct Material
{
	GMfloat Ka[3];
	GMfloat Kd[3];
	GMfloat Ks[3];
	GMfloat shininess;
	ITexture* texture;
};

class Component
{
	friend Object;

public:
	enum
	{
		DefaultEdgesCount = 3,
	};

	Component();
	~Component();

	void setEdgeCountPerPolygon(GMuint edgeCountPerPolygon)
	{
		if (edgeCountPerPolygon > m_edgeCountPerPolygon)
			m_edgeCountPerPolygon = edgeCountPerPolygon;
	}

	GMuint getEdgeCountPerPolygon()
	{
		return m_edgeCountPerPolygon;
	}

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
		return m_verticesCount;
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
		return m_verticesCount / m_edgeCountPerPolygon;
	}

	void generatePolygonProperties();

private:
	GMuint m_verticesCount;
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
	typedef GMfloat DataType;

	Object();
	~Object();

	void disposeMemory();

	void setPainter(ObjectPainter* painter)
	{
		m_painter.reset(painter);
	}

	ObjectPainter* getPainter()
	{
		ASSERT(m_painter);
		return m_painter;
	}

	void appendComponent(AUTORELEASE Component* component, GMuint count);

	std::vector<Component*>& getComponents()
	{
		return m_components;
	}

	std::vector<DataType>& vertices()
	{
		return m_vertices;
	}

	std::vector<DataType>& normals()
	{
		return m_normals;
	}

	std::vector<DataType>& uvs()
	{
		return m_uvs;
	}

	GMuint getBufferId() { return m_bufferId; }
	GMuint getArrayId() { return m_arrayId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }

private:
	std::vector<DataType> m_vertices;
	std::vector<DataType> m_normals;
	std::vector<DataType> m_uvs;
	GMuint m_arrayId;
	GMuint m_bufferId;
	AutoPtr<ObjectPainter> m_painter;
	std::vector<Component*> m_components;
};

END_NS
#endif