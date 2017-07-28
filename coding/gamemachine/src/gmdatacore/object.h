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
	Object* object = nullptr;
};

class GMMesh;
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

// 提供修改缓存的方法
	virtual void beginUpdateBuffer(GMMesh* mesh) = 0;
	virtual void endUpdateBuffer() = 0;
	virtual void* getBuffer() = 0;

protected:
	Object* getObject();
};

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
	void color(GMfloat r, GMfloat g, GMfloat b);
	void endFace();
};

enum class GMUsageHint
{
	StaticDraw,
	DynamicDraw,
};

GM_PRIVATE_OBJECT(Object)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	Vector<GMMesh*> objects;
	AutoPtr<GMObjectPainter> painter;
};

// 所有的顶点属性类型
enum class GMVertexDataType
{
	Position = 0,
	Normal,
	UV,
	Tangent,
	Bitangent,
	Lightmap,
	Color,

	// ---
	EndOfVertexDataType
};

#define gmVertexIndex(i) ((GMuint)i)

GM_ALIGNED_16(class) Object : public GMObject
{
	DECLARE_PRIVATE(Object)

public:
	typedef GMfloat DataType;

public:
	~Object();

public:
	inline void setPainter(AUTORELEASE GMObjectPainter* painter) { D(d); d->painter.reset(painter); }
	inline GMObjectPainter* getPainter() { D(d); return d->painter; }
	inline Vector<GMMesh*>& getAllMeshes() { D(d); return d->objects; }
	inline void append(AUTORELEASE GMMesh* obj) { D(d); d->objects.push_back(obj); }

	// 绘制方式
	void setHint(GMUsageHint hint) { D(d); d->hint = hint; }
	GMUsageHint getHint() { D(d); return d->hint; }
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
	Model = MeshTypeBegin,
	Glyph,
	Particles,
	MeshTypeEnd,
};

#define GM_DEFINE_VERTEX_DATA(name) \
	AlignedVector<Object::DataType> name; \
	GMuint transferred_##name##_byte_size = 0;

#define GM_DEFINE_VERTEX_PROPERTY(name) \
	inline AlignedVector<Object::DataType>& name() { D(d); return d->name; } \
	inline void clear_##name##_and_save_byte_size() {D(d); set_transferred_##name##_byte_size(name().size() * sizeof(Object::DataType)); name().clear(); } \
	inline GMuint get_transferred_##name##_byte_size() { D(d); return d->transferred_##name##_byte_size; } \
	inline void set_transferred_##name##_byte_size(GMuint size) { D(d); d->transferred_##name##_byte_size = size; }

GM_PRIVATE_OBJECT(GMMesh)
{
	GM_DEFINE_VERTEX_DATA(positions);
	GM_DEFINE_VERTEX_DATA(normals);
	GM_DEFINE_VERTEX_DATA(uvs);
	GM_DEFINE_VERTEX_DATA(tangents);
	GM_DEFINE_VERTEX_DATA(bitangents);
	GM_DEFINE_VERTEX_DATA(lightmaps);
	GM_DEFINE_VERTEX_DATA(colors); //顶点颜色，一般渲染不会用到这个，用于粒子绘制

	bool disabledData[gmVertexIndex(GMVertexDataType::EndOfVertexDataType)] = { 0 };
	GMuint arrayId = 0;
	GMuint bufferId = 0;
	Vector<Component*> components;
	GMMeshType type = GMMeshType::Model;
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
	GM_DEFINE_VERTEX_PROPERTY(positions);
	GM_DEFINE_VERTEX_PROPERTY(normals);
	GM_DEFINE_VERTEX_PROPERTY(uvs);
	GM_DEFINE_VERTEX_PROPERTY(tangents);
	GM_DEFINE_VERTEX_PROPERTY(bitangents);
	GM_DEFINE_VERTEX_PROPERTY(lightmaps);
	GM_DEFINE_VERTEX_PROPERTY(colors);

	inline void disableData(GMVertexDataType type) { D(d); d->disabledData[gmVertexIndex(type)] = true; }
	inline bool isDataDisabled(GMVertexDataType type) { D(d); return d->disabledData[gmVertexIndex(type)]; }
	inline Vector<Component*>& getComponents() { D(d); return d->components; }
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