#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include "foundation/vector.h"
#include "image.h"
#include "foundation/utilities/utilities.h"
#include "foundation/linearmath.h"
#include "shader.h"

#define BEGIN_FOREACH_OBJ(obj, mesh) for (auto iter = (obj)->getAllMeshes().begin(); iter != (obj)->getAllMeshes().end(); iter++) { Mesh* mesh = *iter;
#define END_FOREACH_OBJ }

BEGIN_NS

class Object;
class GMGLShaderProgram;

GM_PRIVATE_OBJECT(ObjectPainter)
{
	Object* object;
};

class ObjectPainter : public GMObject
{
	DECLARE_PRIVATE(ObjectPainter)

public:
	ObjectPainter(Object* objs);
	virtual ~ObjectPainter() {}

public:
	virtual void transfer() = 0;
	virtual void draw(GMfloat* modelTransform) = 0;
	virtual void dispose() = 0;
	virtual void clone(Object* obj, OUT ObjectPainter** painter) = 0;

protected:
	Object* getObject();
};

class Mesh;
GM_PRIVATE_OBJECT(Component)
{
	GMuint offset;
	Shader shader;

	// 图元顶点数量
	AlignedVector<GMint> primitiveVertices;
	// 绘制图元数量
	GMuint primitiveCount;
	// 顶点在ChildObject的偏移
	AlignedVector<GMint> vertexOffsets;

	Mesh* parentMesh;
	GMuint currentFaceVerticesCount;
};

class Component : public GMObject
{
	DECLARE_PRIVATE(Component)

	friend class Mesh;

public:
	enum
	{
		DefaultEdgesCount = 3,
	};

	Component(Mesh* parent);
	~Component();

	Shader& getShader()
	{
		D(d);
		return d->shader;
	}

	GMint* getOffsetPtr()
	{
		D(d);
		return d->vertexOffsets.data();
	}

	GMint* getPrimitiveVerticesCountPtr()
	{
		D(d);
		return d->primitiveVertices.data();
	}

	GMuint getPrimitiveCount()
	{
		D(d);
		return d->primitiveCount;
	}

	// suggested methods
	void setVertexOffset(GMuint offset);
	void beginFace();
	void vertex(GMfloat x, GMfloat y, GMfloat z);
	void normal(GMfloat x, GMfloat y, GMfloat z);
	void uv(GMfloat u, GMfloat v);
	void lightmap(GMfloat u, GMfloat v);
	void endFace();
};

GM_PRIVATE_OBJECT(Object)
{
	AlignedVector<Mesh*> objects;
	AutoPtr<ObjectPainter> painter;
};

GM_ALIGNED_16(class) Object : public GMObject
{
	DECLARE_PRIVATE(Object)

public:
	typedef GMfloat DataType;

public:
	~Object();

public:
	void setPainter(AUTORELEASE ObjectPainter* painter)
	{
		D(d);
		d->painter.reset(painter);
	}

	ObjectPainter* getPainter()
	{
		D(d);
		return d->painter;
	}

	AlignedVector<Mesh*>& getAllMeshes()
	{
		D(d);
		return d->objects;
	}

	void append(AUTORELEASE Mesh* obj)
	{
		D(d);
		d->objects.push_back(obj);
	}
};

class Mesh
{
	friend class Object_Less;

public:
	enum MeshesType
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

	Mesh();
	Mesh(const std::string& name);
	~Mesh();

	void clone(OUT Mesh** childObject);

	void appendComponent(AUTORELEASE Component* component);

	void calculateTangentSpace();

	AlignedVector<AUTORELEASE Component*>& getComponents()
	{
		return m_components;
	}

	AlignedVector<Object::DataType>& vertices()
	{
		return m_vertices;
	}

	AlignedVector<Object::DataType>& normals()
	{
		return m_normals;
	}

	AlignedVector<Object::DataType>& uvs()
	{
		return m_uvs;
	}

	AlignedVector<Object::DataType>& tangents()
	{
		return m_tangents;
	}

	AlignedVector<Object::DataType>& bitangents()
	{
		return m_bitangents;
	}

	AlignedVector<Object::DataType>& lightmaps()
	{
		return m_lightmaps;
	}

	MeshesType getType()
	{
		return m_type;
	}

	void setType(MeshesType type)
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
	AlignedVector<Object::DataType> m_vertices;
	AlignedVector<Object::DataType> m_normals;
	AlignedVector<Object::DataType> m_uvs;
	AlignedVector<Object::DataType> m_tangents;
	AlignedVector<Object::DataType> m_bitangents;
	AlignedVector<Object::DataType> m_lightmaps;
	GMuint m_arrayId;
	GMuint m_bufferId;
	AlignedVector<Component*> m_components;
	MeshesType m_type;
	ArrangementMode m_mode;
	std::string m_name;
};

END_NS
#endif