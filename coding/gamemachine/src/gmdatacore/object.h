#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include "foundation/vector.h"
#include "image.h"
#include "foundation/utilities/utilities.h"
#include "foundation/linearmath.h"
#include "shader.h"

#define BEGIN_FOREACH_OBJ(obj, mesh) for (auto iter = (obj)->getAllMeshes().begin(); iter != (obj)->getAllMeshes().end(); iter++) { GMMesh* mesh = *iter;
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

class GMMesh;
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

	GMMesh* parentMesh;
	GMuint currentFaceVerticesCount;
};

class Component : public GMObject
{
	DECLARE_PRIVATE(Component)

	friend class GMMesh;

public:
	enum
	{
		DefaultEdgesCount = 3,
	};

	Component(GMMesh* parent);
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
	AlignedVector<GMMesh*> objects;
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

	AlignedVector<GMMesh*>& getAllMeshes()
	{
		D(d);
		return d->objects;
	}

	void append(AUTORELEASE GMMesh* obj)
	{
		D(d);
		d->objects.push_back(obj);
	}
};


// 绘制时候的排列方式
enum class GMArrangementMode
{
	// 默认排列，按照Components，并按照一个个三角形来画
	Triangle_Fan,

	Triangle_Strip,

	Triangles,
};

enum class GMMeshType
{
	MeshTypeBegin,
	Normal = MeshTypeBegin,
	Sky,
	Glyph,
	MeshTypeEnd,
};

GM_PRIVATE_OBJECT(GMMesh)
{
	AlignedVector<Object::DataType> vertices;
	AlignedVector<Object::DataType> normals;
	AlignedVector<Object::DataType> uvs;
	AlignedVector<Object::DataType> tangents;
	AlignedVector<Object::DataType> bitangents;
	AlignedVector<Object::DataType> lightmaps;
	GMuint arrayId = 0;
	GMuint bufferId = 0;
	AlignedVector<Component*> components;
	GMMeshType type = GMMeshType::Normal;
	GMArrangementMode mode = GMArrangementMode::Triangle_Fan;
	GMString name = _L("default");
};

class GMMesh : public GMObject
{
	DECLARE_PRIVATE(GMMesh)

	friend class Object_Less;

public:
	GMMesh();
	~GMMesh();

public:
	void clone(OUT GMMesh** childObject);
	void appendComponent(AUTORELEASE Component* component);
	void calculateTangentSpace();

public:
	AlignedVector<AUTORELEASE Component*>& getComponents()
	{
		D(d);
		return d->components;
	}

	AlignedVector<Object::DataType>& vertices()
	{
		D(d);
		return d->vertices;
	}

	AlignedVector<Object::DataType>& normals()
	{
		D(d);
		return d->normals;
	}

	AlignedVector<Object::DataType>& uvs()
	{
		D(d);
		return d->uvs;
	}

	AlignedVector<Object::DataType>& tangents()
	{
		D(d);
		return d->tangents;
	}

	AlignedVector<Object::DataType>& bitangents()
	{
		D(d);
		return d->bitangents;
	}

	AlignedVector<Object::DataType>& lightmaps()
	{
		D(d);
		return d->lightmaps;
	}

	GMMeshType getType()
	{
		D(d);
		return d->type;
	}

	void setType(GMMeshType type)
	{
		D(d);
		d->type = type;
	}

	void setArrangementMode(GMArrangementMode mode)
	{
		D(d);
		d->mode = mode;
	}

	GMArrangementMode getArrangementMode()
	{
		D(d);
		return d->mode;
	}

	void setName(const char* name)
	{
		D(d);
		d->name = name;
	}

	const GMString& getName()
	{
		D(d);
		return d->name;
	}

	GMuint getBufferId()
	{
		D(d);
		return d->bufferId;
	}

	GMuint getArrayId()
	{
		D(d);
		return d->arrayId;
	}

	void setBufferId(GMuint id)
	{
		D(d);
		d->bufferId = id;
	}

	void setArrayId(GMuint id)
	{
		D(d);
		d->arrayId = id;
	}
};

END_NS
#endif