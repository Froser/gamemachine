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

GM_PRIVATE_OBJECT(GMObjectPainter)
{
	Object* object;
};

class GMObjectPainter : public GMObject
{
	DECLARE_PRIVATE(GMObjectPainter)

public:
	GMObjectPainter(Object* obj)
	{
		D(d);
		d->object = obj;
	}

	virtual ~GMObjectPainter() {}

public:
	virtual void transfer() = 0;
	virtual void draw(GMfloat* modelTransform) = 0;
	virtual void dispose() = 0;

protected:
	Object* getObject();
};

class GMMesh;
GM_PRIVATE_OBJECT(Component)
{
	GMuint offset = 0;
	// 绘制图元数量
	GMuint primitiveCount = 0;

	// 图元顶点数量
	Vector<GMint> primitiveVertices;
	// 顶点在ChildObject的偏移
	Vector<GMint> vertexOffsets;

	GMMesh* parentMesh;
	GMuint currentFaceVerticesCount;
	Shader shader;
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

	inline Shader& getShader() { D(d); return d->shader; }
	inline void setShader(const Shader& shader) { D(d); d->shader = shader; }
	inline GMint* getOffsetPtr() { D(d); return d->vertexOffsets.data(); }
	inline GMint* getPrimitiveVerticesCountPtr() { D(d); return d->primitiveVertices.data(); }
	inline GMuint getPrimitiveCount() { D(d); return d->primitiveCount; }

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
	Vector<GMMesh*> objects;
	AutoPtr<GMObjectPainter> painter;
};

GM_ALIGNED_16(class) Object : public GMObject
{
	DECLARE_PRIVATE(Object)

public:
	typedef GMfloat DataType;

public:
	~Object();

public:
	inline void setPainter(AUTORELEASE GMObjectPainter* painter)
	{
		D(d);
		d->painter.reset(painter);
	}

	inline GMObjectPainter* getPainter()
	{
		D(d);
		return d->painter;
	}

	inline Vector<GMMesh*>& getAllMeshes()
	{
		D(d);
		return d->objects;
	}

	inline void append(AUTORELEASE GMMesh* obj)
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

	Lines,
};

enum class GMMeshType
{
	MeshTypeBegin,
	Normal = MeshTypeBegin,
	Glyph,
	Particles,
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
	AlignedVector<Object::DataType> colors; //顶点坐标，一般渲染不会用到这个，用于粒子绘制
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
	inline AlignedVector<AUTORELEASE Component*>& getComponents() { D(d); return d->components; }
	inline AlignedVector<Object::DataType>& vertices() { D(d); return d->vertices; }
	inline AlignedVector<Object::DataType>& normals() { D(d); return d->normals; }
	inline AlignedVector<Object::DataType>& uvs() { D(d); return d->uvs; }
	inline AlignedVector<Object::DataType>& tangents() { D(d); return d->tangents; }
	inline AlignedVector<Object::DataType>& bitangents() { D(d); return d->bitangents; }
	inline AlignedVector<Object::DataType>& lightmaps() { D(d); return d->lightmaps; }
	inline AlignedVector<Object::DataType>& colors() { D(d); return d->colors; }
	inline GMMeshType getType() { D(d); return d->type; }
	inline void setType(GMMeshType type) { D(d); d->type = type; }
	inline void setArrangementMode(GMArrangementMode mode) { D(d); d->mode = mode; }
	inline GMArrangementMode getArrangementMode() { D(d); return d->mode; }
	inline void setName(const char* name) { D(d); d->name = name; }
	inline const GMString& getName() { D(d); return d->name; }
	inline GMuint getBufferId() { D(d); return d->bufferId; }
	inline GMuint getArrayId() { D(d); return d->arrayId; }
	inline void setBufferId(GMuint id) { D(d); d->bufferId = id; }
	inline void setArrayId(GMuint id) { D(d); d->arrayId = id; }
};

END_NS
#endif