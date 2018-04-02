#ifndef __GMMODEL_H__
#define __GMMODEL_H__
#include <gmcommon.h>
#include <tools.h>
#include <linearmath.h>
#include <gmimage.h>
#include <gmshader.h>
#include <atomic>

BEGIN_NS

class GMModel;
class GMGameObject;

struct GMVertex
{
	Array<GMfloat, 3> vertices;
	Array<GMfloat, 3> normals;
	Array<GMfloat, 2> texcoords;
	Array<GMfloat, 3> tangents;
	Array<GMfloat, 3> bitangents;
	Array<GMfloat, 2> lightmaps;
	Array<GMfloat, 4> color;
};

GM_PRIVATE_OBJECT(GMModelPainter)
{
	GMModel* model = nullptr;
};

class GMMesh;
class GMMeshData;
class GMModelPainter : public GMObject, public IQueriable
{
	DECLARE_PRIVATE(GMModelPainter)

public:
	GMModelPainter(GMModel* obj)
	{
		D(d);
		d->model = obj;
	}

public:
	virtual void transfer() = 0;
	virtual void draw(const GMGameObject* parent) = 0;
	virtual void dispose(GMMeshData* md) = 0;

// 提供修改缓存的方法
	virtual void beginUpdateBuffer(GMMesh* mesh) = 0;
	virtual void endUpdateBuffer() = 0;
	virtual void* getBuffer() = 0;

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) { return false; }
	virtual bool setInterface(GameMachineInterfaceID id, void* in) { return false; }

protected:
	inline GMModel* getModel() { D(d); return d->model; }

protected:
	void packData(Vector<GMVertex>& packedData);
};

GM_PRIVATE_OBJECT(GMComponent)
{
	GMuint offset = 0;
	// 绘制图元数量
	GMuint primitiveCount = 0;
	GMuint verticesCount = 0;

	// 图元顶点数量
	AlignedVector<GMuint> primitiveVertices;
	// 顶点在ChildObject的偏移
	AlignedVector<GMuint> vertexOffsets;

	GMMesh* parentMesh = nullptr;
	GMuint currentFaceVerticesCount;
	GMShader shader;
};

class GMComponent : public GMObject
{
	DECLARE_PRIVATE(GMComponent)
	GM_ALLOW_COPY_DATA(GMComponent)

	friend class GMMesh;
	friend class GMModel;

public:
	GMComponent(GMMesh* parent);

	inline GMShader& getShader() { D(d); return d->shader; }
	inline void setShader(const GMShader& shader) { D(d); d->shader = shader; }
	inline GMuint* getOffsetPtr() { D(d); return d->vertexOffsets.data(); }
	inline GMuint* getPrimitiveVerticesCountPtr() { D(d); return d->primitiveVertices.data(); }
	inline GMuint getPrimitiveCount() { D(d); return d->primitiveCount; }
	inline GMuint getVerticesCount() { D(d); return d->verticesCount; }

	void clear();
	void setVertexOffset(GMuint offset);
	void beginFace();
	void vertex(GMfloat x, GMfloat y, GMfloat z);
	void normal(GMfloat x, GMfloat y, GMfloat z);
	void texcoord(GMfloat u, GMfloat v);
	void lightmap(GMfloat u, GMfloat v);
	void color(GMfloat r, GMfloat g, GMfloat b, GMfloat a = 1.0f);
	void endFace();

private:
	void setParentMesh(GMMesh* mesh) { D(d); d->parentMesh = mesh; }
};

enum class GMUsageHint
{
	StaticDraw,
	DynamicDraw,
};

// 和着色器中的GM_shader_type一致
enum class GMModelType
{
	ModelTypeBegin,
	Model2D = ModelTypeBegin,
	Model3D,
	Particles,
	Glyph,
	CubeMap,
	ModelTypeEnd,
};

GM_PRIVATE_OBJECT(GMModel)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	GMMesh* mesh = nullptr;
	GMScopePtr<GMModelPainter> painter;
	GMModelType type = GMModelType::Model3D;
	bool needTransfer = true;
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

class GMModel : public GMObject
{
	DECLARE_PRIVATE(GMModel)

public:
	typedef GMfloat DataType;

	enum Dimensions
	{
		PositionDimension = 3,
		NormalDimension = 3,
		TexcoordDimension = 2,
		TextureDimension = 4,
		LightmapDimension = TexcoordDimension,
		TangentDimension = NormalDimension,
		BitangentDimension = NormalDimension,
	};

public:
	GMModel();
	//! 通过另外一个GMModel，构造一个GMModel。
	/*!
	  被构造的GMModel将与原本的GMModel共享一份顶点缓存，但是，其component和原本的模型的component是分开的副本。
	  \param model 构造的模型的原型。如果此模型顶点处于未传输状态，其顶点数据将会被强行传输。
	*/
	GMModel(GMModel& model);
	~GMModel();

public:
	inline void setPainter(AUTORELEASE GMModelPainter* painter) { D(d); d->painter.reset(painter); }
	inline GMModelPainter* getPainter() { D(d); return d->painter; }
	inline GMMesh* getMesh() { D(d); return d->mesh; }
	inline const GMMesh* getMesh() const { D(d); return d->mesh; }
	inline GMModelType getType() { D(d); return d->type; }
	inline void setType(GMModelType type) { D(d); d->type = type; }

	//! 表示此模型是否需要被GMModelPainter将顶点数据传输到显卡。
	/*!
	  如果一个模型第一次建立顶点数据，则需要将这些数据传输到显卡。<br/>
	  然而，如果此模型如果与其他模型共享一份顶点数据，那么此模型不需要传输顶点数据到显卡，因为数据已经存在。
	  \sa GMModelPainter()
	*/
	inline bool isNeedTransfer() { D(d); return d->needTransfer; }

	//! 表示此模型不再需要将顶点数据传输到显卡了。
	/*!
	  当使用了已经传输过的顶点数据，或者顶点数据传输完成时调用此方法。
	*/
	inline void needNotTransferAnymore() { D(d); d->needTransfer = false; }

	// 绘制方式
	void setUsageHint(GMUsageHint hint) { D(d); d->hint = hint; }
	GMUsageHint getUsageHint() { D(d); return d->hint; }

	void releaseMesh();
};

// 绘制时候的排列方式
enum class GMArrangementMode
{
	// 默认排列，按照Components，并按照一个个三角形来画
	TriangleStrip,
	Triangles,
	Lines,
};

class GMMesh;
struct GMMeshBuffer
{
	union
	{
		struct //OpenGL
		{
			GMuint arrayId;
			GMuint bufferId;
		};

		void* buffer; //DirectX
	};
};

GM_PRIVATE_OBJECT(GMMeshData)
{
	GMMeshBuffer buffer = { 0 };
	std::atomic<GMint> ref;
	GMModelPainter* painter = nullptr;
};

class GMMeshData : public GMObject
{
	DECLARE_PRIVATE(GMMeshData)
	friend class GMMesh;

	GMMeshData();
	~GMMeshData();

	void dispose();
	void setMeshBuffer(const GMMeshBuffer& meshBuffer)
	{
		D(d);
		d->buffer = meshBuffer;
	}

	const GMMeshBuffer& getMeshBuffer()
	{
		D(d);
		return d->buffer;
	}

	void addRef()
	{
		D(d);
		++d->ref;
	}

	void releaseRef()
	{
		D(d);
		--d->ref;
		if (hasNoRef())
			dispose();
	}

	bool hasNoRef()
	{
		D(d);
		return d->ref == 0;
	}
};

#define GM_DEFINE_VERTEX_DATA(name) \
	Vector<GMModel::DataType> name; \
	GMuint transferred_##name##_byte_size = 0;

#define GM_DEFINE_VERTEX_PROPERTY(name) \
	inline auto& name() { D(d); return d->name; } \
	inline void clear_##name##_and_save_byte_size() {D(d); set_transferred_##name##_byte_size(name().size() * sizeof(GMModel::DataType)); name().clear(); } \
	inline GMuint get_transferred_##name##_byte_size() { D(d); return d->transferred_##name##_byte_size; } \
	inline void set_transferred_##name##_byte_size(GMuint size) { D(d); d->transferred_##name##_byte_size = size; }

#define GM_COPY_VERTEX_PROPERTY(to_ptr, from_ptr, name) \
	(to_ptr)->set_transferred_##name##_byte_size((from_ptr)->get_transferred_##name##_byte_size());

GM_PRIVATE_OBJECT(GMMesh)
{
	GM_DEFINE_VERTEX_DATA(positions);
	GM_DEFINE_VERTEX_DATA(normals);
	GM_DEFINE_VERTEX_DATA(texcoords);
	GM_DEFINE_VERTEX_DATA(tangents);
	GM_DEFINE_VERTEX_DATA(bitangents);
	GM_DEFINE_VERTEX_DATA(lightmaps);
	GM_DEFINE_VERTEX_DATA(colors); //顶点颜色，一般渲染不会用到这个，用于粒子绘制

	GMMeshData* meshData = nullptr;
	Vector<GMComponent*> components;
	GMArrangementMode mode = GMArrangementMode::Triangles;
	GMString name = L"default";
};

class GMMesh : public GMObject
{
	DECLARE_PRIVATE(GMMesh)

	friend class GMModel;
	friend class GMComponent;

private:
	GMMesh();

public:
	~GMMesh();

public:
	void clone(OUT GMMesh** childObject);
	void calculateTangentSpace();

private:
	void appendComponent(AUTORELEASE GMComponent* component);

public:
	GM_DEFINE_VERTEX_PROPERTY(positions);
	GM_DEFINE_VERTEX_PROPERTY(normals);
	GM_DEFINE_VERTEX_PROPERTY(texcoords);
	GM_DEFINE_VERTEX_PROPERTY(tangents);
	GM_DEFINE_VERTEX_PROPERTY(bitangents);
	GM_DEFINE_VERTEX_PROPERTY(lightmaps);
	GM_DEFINE_VERTEX_PROPERTY(colors);

	inline Vector<GMComponent*>& getComponents() { D(d); return d->components; }
	inline const Vector<GMComponent*>& getComponents() const { D(d); return d->components; }
	inline void setArrangementMode(GMArrangementMode mode) { D(d); d->mode = mode; }
	inline GMArrangementMode getArrangementMode() { D(d); return d->mode; }
	inline void setName(const GMString& name) { D(d); d->name = name; }
	inline const GMString& getName() { D(d); return d->name; }
	inline const GMMeshBuffer& getMeshBuffer() { D(d); return d->meshData->getMeshBuffer(); }
	inline void setMeshBuffer(const GMMeshBuffer& buffer) { D(d); d->meshData->setMeshBuffer(buffer); }

public:
	//! 释放一个网格数据
	/*!
	  将网格所绑定的数据的引用计数减一，如果网格数据引用计数为0，则从GPU中删除顶点数据，并析构此网格数据，且将其置为空。
	*/
	void releaseMeshData();

private:
	inline GMMeshData* getMeshData() { D(d); return d->meshData; }
	inline const GMMeshData* getMeshData() const { D(d); return d->meshData; }
	inline void setMeshData(GMMeshData* md);
};

#define IF_ENABLED(mesh, type) if (!mesh->isDataDisabled(type))

END_NS
#endif