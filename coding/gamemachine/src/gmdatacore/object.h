#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <vector>
#include "image.h"
#include "utilities/assert.h"
#include "utilities/autoptr.h"
#include "utilities/vmath.h"
#include "gmdatacore/texture.h"
#include "shader.h"

#define BEGIN_FOREACH_OBJ(obj, childObj) for (auto iter = (obj)->getChildObjects().begin(); iter != (obj)->getChildObjects().end(); iter++) { ChildObject* childObj = *iter;
#define END_FOREACH_OBJ }

BEGIN_NS

class Object;
class GMGLShaders;
class ObjectPainter
{
public:
	ObjectPainter(Object* objs);

public:
	virtual void transfer() = 0;
	virtual void draw() = 0;
	virtual void dispose() = 0;
	virtual void clone(Object* obj, OUT ObjectPainter** painter) = 0;

protected:
	Object* getObject();

private:
	Object* m_object;
};

class ChildObject;
class Component
{
	friend class ChildObject;

public:
	enum
	{
		DefaultEdgesCount = 3,
	};

	Component(ChildObject* parent);
	~Component();

	Shader& getShader()
	{
		return m_shader;
	}

	GMint* getOffsetPtr()
	{
		return m_vertexOffsets.data();
	}

	GMint* getPrimitiveVerticesCountPtr()
	{
		return m_primitiveVertices.data();
	}

	GMuint getPrimitiveCount()
	{
		return m_primitiveCount;
	}

	// suggested methods
	void setVertexOffset(GMuint offset);
	void beginFace();
	void vertex(GMfloat x, GMfloat y, GMfloat z);
	void normal(GMfloat x, GMfloat y, GMfloat z);
	void uv(GMfloat u, GMfloat v);
	void lightmap(GMfloat u, GMfloat v);
	void endFace();

private:
	GMuint m_offset;
	Shader m_shader;

	// 图元顶点数量
	std::vector<GMint> m_primitiveVertices;
	// 绘制图元数量
	GMuint m_primitiveCount;
	// 顶点在ChildObject的偏移
	std::vector<GMint> m_vertexOffsets;

	ChildObject* m_parent;
	GMuint m_currentFaceVerticesCount;
};

class Object
{
public:
	typedef GMfloat DataType;

public:
	~Object();

public:
	void setPainter(AUTORELEASE ObjectPainter* painter)
	{
		m_painter.reset(painter);
	}

	ObjectPainter* getPainter()
	{
		return m_painter;
	}

	std::vector<ChildObject*>& getChildObjects()
	{
		return m_objects;
	}

	void append(AUTORELEASE ChildObject* obj)
	{
		m_objects.push_back(obj);
	}

private:
	AutoPtr<ObjectPainter> m_painter;
	std::vector<ChildObject*> m_objects;
};

class ChildObject
{
	friend class Object_Less;

public:
	enum ObjectType
	{
		ObjectTypeBegin,
		NormalObject = ObjectTypeBegin,
		Sky,
		Glyph,
		ObjectTypeEnd,
	};

	// 绘制时候的排列方式
	enum ArrangementMode
	{
		// 默认排列，按照Components，并按照一个个三角形来画
		Triangle_Fan,

		Triangle_Strip,

		Triangles,
	};

	ChildObject();
	ChildObject(const std::string& name);
	~ChildObject();

	void clone(OUT ChildObject** childObject);

	void appendComponent(AUTORELEASE Component* component);

	void calculateTangentSpace();

	std::vector<AUTORELEASE Component*>& getComponents()
	{
		return m_components;
	}

	std::vector<Object::DataType>& vertices()
	{
		return m_vertices;
	}

	std::vector<Object::DataType>& normals()
	{
		return m_normals;
	}

	std::vector<Object::DataType>& uvs()
	{
		return m_uvs;
	}

	std::vector<Object::DataType>& tangents()
	{
		return m_tangents;
	}

	std::vector<Object::DataType>& bitangents()
	{
		return m_bitangents;
	}

	std::vector<Object::DataType>& lightmaps()
	{
		return m_lightmaps;
	}

	ObjectType getType()
	{
		return m_type;
	}

	void setType(ObjectType type)
	{
		m_type = type;
	}

	void setArrangementMode(ArrangementMode mode)
	{
		m_mode = mode;
	}

	ArrangementMode getArrangementMode()
	{
		return m_mode;
	}

	void setName(const char* name)
	{
		m_name = name;
	}

	const std::string& getName()
	{
		return m_name;
	}

	GMuint getBufferId() { return m_bufferId; }
	GMuint getArrayId() { return m_arrayId; }
	void setBufferId(GMuint id) { m_bufferId = id; }
	void setArrayId(GMuint id) { m_arrayId = id; }

private:
	std::vector<Object::DataType> m_vertices;
	std::vector<Object::DataType> m_normals;
	std::vector<Object::DataType> m_uvs;
	std::vector<Object::DataType> m_tangents;
	std::vector<Object::DataType> m_bitangents;
	std::vector<Object::DataType> m_lightmaps;
	GMuint m_arrayId;
	GMuint m_bufferId;
	std::vector<Component*> m_components;
	ObjectType m_type;
	ArrangementMode m_mode;
	std::string m_name;
};

END_NS
#endif