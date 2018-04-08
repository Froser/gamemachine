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
	enum Dimensions
	{
		PositionDimension = 3,
		NormalDimension = 3,
		TexcoordDimension = 2,
		TextureDimension = 3,
		LightmapDimension = TexcoordDimension,
		TangentDimension = NormalDimension,
		BitangentDimension = NormalDimension,
		ColorDimension = 4,
	};

	Array<GMfloat, PositionDimension> vertices;
	Array<GMfloat, NormalDimension> normals;
	Array<GMfloat, TexcoordDimension> texcoords;
	Array<GMfloat, TextureDimension> tangents;
	Array<GMfloat, BitangentDimension> bitangents;
	Array<GMfloat, LightmapDimension> lightmaps;
	Array<GMfloat, ColorDimension> color;
};

typedef Vector<GMMesh*> GMMeshes;

GM_PRIVATE_OBJECT(GMModelPainter)
{
	GMModel* model = nullptr;
};

class GMMesh;
class GMModelBuffer;
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
	virtual void dispose(GMModelBuffer* md) = 0;

// 提供修改缓存的方法
	virtual void beginUpdateBuffer(GMModel* model) = 0;
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

class GMMesh;
struct GMModelBufferData
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

GM_PRIVATE_OBJECT(GMModelBuffer)
{
	GMModelBufferData buffer = { 0 };
	std::atomic<GMint> ref;
	GMModelPainter* painter = nullptr;
};

//! 用来管理GMModelBuffer生命周期的类，包含引用计数功能。
class GMModelBuffer : public GMObject
{
	DECLARE_PRIVATE(GMModelBuffer)

	GMModelBuffer();
	~GMModelBuffer();

	void dispose();
	void setData(const GMModelBufferData& bufferData)
	{
		D(d);
		d->buffer = bufferData;
	}

	const GMModelBufferData& getMeshBuffer()
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
		return d->ref <= 0;
	}
};

// 绘制时候的排列方式
enum class GMTopologyMode
{
	// 默认排列，按照Components，并按照一个个三角形来画
	TriangleStrip,
	Triangles,
	Lines,
};

GM_PRIVATE_OBJECT(GMModel)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	GMMeshes meshes;
	GMScopePtr<GMModelPainter> painter;
	GMShader shader;
	GMModelBuffer* modelBuffer = nullptr;
	GMModelType type = GMModelType::Model3D;
	GMTopologyMode mode = GMTopologyMode::Triangles;
	GMuint verticesCount = 0;
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

public:
	GMModel();
	GMModel(GMModel& model);
	~GMModel();

	GM_DECLARE_PROPERTY(PrimitiveTopologyMode, mode, GMTopologyMode);
	GM_DECLARE_PROPERTY(Type, type, GMModelType);
	GM_DECLARE_PROPERTY(Shader, shader, GMShader);
	GM_DECLARE_PROPERTY(VerticesCount, verticesCount, GMuint);

public:
	inline void setPainter(AUTORELEASE GMModelPainter* painter)
	{
		D(d);
		d->painter.reset(painter);
	}

	inline GMModelPainter* getPainter()
	{
		D(d);
		return d->painter;
	}

	inline void addMesh(GMMesh* mesh)
	{
		D(d);
		d->meshes.push_back(mesh);
	}

	inline GMMeshes& getMeshes()
	{
		D(d);
		return d->meshes;
	}

	inline const GMModelBufferData* getBuffer()
	{
		D(d);
		if (!d->modelBuffer)
			return nullptr;
		return &d->modelBuffer->getMeshBuffer();
	}

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

	void setModelBuffer(AUTORELEASE GMModelBuffer* mb);

	void releaseModelBuffer();
};

#define GM_DEFINE_VERTEX_DATA(name) \
	Vector<GMModel::DataType> name; \
	GMuint transferred_##name##_byte_size = 0;

#define GM_DEFINE_VERTEX_PROPERTY(name) \
	inline auto& name() { D(d); return d->name; }

GM_PRIVATE_OBJECT(GMMesh)
{
	GM_DEFINE_VERTEX_DATA(positions);
	GM_DEFINE_VERTEX_DATA(normals);
	GM_DEFINE_VERTEX_DATA(texcoords);
	GM_DEFINE_VERTEX_DATA(tangents);
	GM_DEFINE_VERTEX_DATA(bitangents);
	GM_DEFINE_VERTEX_DATA(lightmaps);
	GM_DEFINE_VERTEX_DATA(colors); //顶点颜色，一般渲染不会用到这个，用于粒子绘制

	bool firstFace = true;
	GMuint verticesPerFace = 0;
	GMuint currentFaceVerticesCount = 0;
};

//! 表示一份网格数据。
/*!
  网格数据可能仅仅是模型中的一段数据。
*/
class GMMesh : public GMObject
{
	DECLARE_PRIVATE(GMMesh)

public:
	GMMesh(GMModel* parent);

public:
	GM_DEFINE_VERTEX_PROPERTY(positions);
	GM_DEFINE_VERTEX_PROPERTY(normals);
	GM_DEFINE_VERTEX_PROPERTY(texcoords);
	GM_DEFINE_VERTEX_PROPERTY(tangents);
	GM_DEFINE_VERTEX_PROPERTY(bitangents);
	GM_DEFINE_VERTEX_PROPERTY(lightmaps);
	GM_DEFINE_VERTEX_PROPERTY(colors);

	void calculateTangentSpace();
	void clear();
	void beginFace();
	void vertex(GMfloat x, GMfloat y, GMfloat z);
	void normal(GMfloat x, GMfloat y, GMfloat z);
	void texcoord(GMfloat u, GMfloat v);
	void lightmap(GMfloat u, GMfloat v);
	void color(GMfloat r, GMfloat g, GMfloat b, GMfloat a = 1.0f);
	void endFace();

public:
	inline GMuint getVerticesPerFace()
	{
		D(d);
		return d->verticesPerFace;
	}
};

END_NS
#endif