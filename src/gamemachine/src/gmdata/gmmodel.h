#ifndef __GMMODEL_H__
#define __GMMODEL_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <linearmath.h>
#include <gmimage.h>
#include <gmshader.h>
#include <atomic>
#include <gmskeleton.h>

struct ID3D11Buffer;

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
		LightmapDimension = TexcoordDimension,
		TangentDimension = NormalDimension,
		BitangentDimension = NormalDimension,
		ColorDimension = 4,
	};

	Array<GMfloat, PositionDimension> positions;
	Array<GMfloat, NormalDimension> normals;
	Array<GMfloat, TexcoordDimension> texcoords;
	Array<GMfloat, TangentDimension> tangents;
	Array<GMfloat, BitangentDimension> bitangents;
	Array<GMfloat, LightmapDimension> lightmaps;
	Array<GMfloat, ColorDimension> color;
};

typedef Vector<GMMesh*> GMMeshes;

enum class GMModelBufferType
{
	VertexBuffer,
	IndexBuffer,
};

GM_PRIVATE_OBJECT(GMModelDataProxy)
{
	const IRenderContext* context = nullptr;
	GMModel* model = nullptr;
};

class GMMesh;
class GMModelBuffer;
class GMModelDataProxy : public GMObject, public IQueriable
{
	GM_DECLARE_PRIVATE(GMModelDataProxy)

public:
	GMModelDataProxy(const IRenderContext* context, GMModel* obj)
	{
		D(d);
		d->context = context;
		d->model = obj;
	}

public:
	virtual void transfer() = 0;
	virtual void dispose(GMModelBuffer* md) = 0;

// 提供修改缓存的方法
	virtual void beginUpdateBuffer(GMModelBufferType type = GMModelBufferType::VertexBuffer) = 0;
	virtual void endUpdateBuffer() = 0;
	virtual void* getBuffer() = 0;

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) { return false; }
	virtual bool setInterface(GameMachineInterfaceID id, void* in) { return false; }

	virtual const IRenderContext* getContext();

protected:
	inline GMModel* getModel() { D(d); return d->model; }

protected:
	void prepareTangentSpace();
	void packVertices(Vector<GMVertex>& vertices);
	void packIndices(Vector<GMuint>& indices);
};

enum class GMUsageHint
{
	StaticDraw,
	DynamicDraw,
};

class GMMesh;
struct GMModelBufferData
{
	union
	{
		struct //OpenGL
		{
			GMuint arrayId;
			GMuint vertexBufferId;
			GMuint indexBufferId;
		};

		struct //DirectX
		{
			ID3D11Buffer* vertexBuffer;
			ID3D11Buffer* indexBuffer;
		};
	};
};

GM_PRIVATE_OBJECT(GMModelBuffer)
{
	GMModelBufferData buffer = { 0 };
	GMAtomic<GMint> ref;
	GMModelDataProxy* modelDataProxy = nullptr;
};

//! 用来管理GMModelBuffer生命周期的类，包含引用计数功能。
class GMModelBuffer : public GMObject
{
	GM_DECLARE_PRIVATE(GMModelBuffer)

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
		{
			dispose();
			delete this;
		}
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

enum class GMModelDrawMode
{
	Vertex,
	Index,
};

GM_PRIVATE_OBJECT(GMModel)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	GMMeshes meshes;
	GMOwnedPtr<GMModelDataProxy> modelDataProxy;
	GMShader shader;
	GMModelBuffer* modelBuffer = nullptr;
	GMModelDrawMode drawMode = GMModelDrawMode::Vertex;
	GMModelType type = GMModelType::Model3D;
	GMTopologyMode mode = GMTopologyMode::Triangles;
	GMsize_t verticesCount = 0;
	bool needTransfer = true;
};

// 所有的顶点属性类型
enum class GMVertexDataType
{
	Position = 0,
	Normal,
	Texcoord,
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
	GM_DECLARE_PRIVATE(GMModel)

public:
	typedef GMfloat DataType;

public:
	GMModel();
	GMModel(GMModel& model);
	~GMModel();

	GM_DECLARE_PROPERTY(PrimitiveTopologyMode, mode, GMTopologyMode);
	GM_DECLARE_PROPERTY(Type, type, GMModelType);
	GM_DECLARE_PROPERTY(Shader, shader, GMShader);
	GM_DECLARE_PROPERTY(VerticesCount, verticesCount, GMsize_t);
	GM_DECLARE_PROPERTY(DrawMode, drawMode, GMModelDrawMode);

public:
	inline void setModelDataProxy(AUTORELEASE GMModelDataProxy* modelDataProxy)
	{
		D(d);
		d->modelDataProxy.reset(modelDataProxy);
	}

	inline GMModelDataProxy* getModelDataProxy() GM_NOEXCEPT
	{
		D(d);
		return d->modelDataProxy.get();
	}

	inline GMMeshes& getMeshes() GM_NOEXCEPT
	{
		D(d);
		return d->meshes;
	}

	//! 表示此模型是否需要被GMModelDataProxy将顶点数据传输到显卡。
	/*!
	  如果一个模型第一次建立顶点数据，则需要将这些数据传输到显卡。<br/>
	  然而，如果此模型如果与其他模型共享一份顶点数据，那么此模型不需要传输顶点数据到显卡，因为数据已经存在。
	  \sa GMModelDataProxy()
	*/
	inline bool isNeedTransfer() GM_NOEXCEPT { D(d); return d->needTransfer; }

	//! 表示此模型不再需要将顶点数据传输到显卡了。
	/*!
	  当使用了已经传输过的顶点数据，或者顶点数据传输完成时调用此方法。
	*/
	inline void doNotTransferAnymore() GM_NOEXCEPT
	{
		D(d);
		d->needTransfer = false;
	}

	// 绘制方式
	void setUsageHint(GMUsageHint hint) GM_NOEXCEPT
	{
		D(d);
		d->hint = hint;
	}

	GMUsageHint getUsageHint()
	{
		D(d);
		return d->hint;
	}

	void setModelBuffer(AUTORELEASE GMModelBuffer* mb);
	GMModelBuffer* getModelBuffer();
	void releaseModelBuffer();
	void addMesh(GMMesh* mesh);
};

#define GM_DEFINE_VERTEX_DATA(name) \
	Vector<GMModel::DataType> name;

#define GM_DEFINE_VERTEX_PROPERTY(name) \
	inline auto& name() { D(d); return d->name; }


GM_PRIVATE_OBJECT(GMModels)
{
	Vector<GMAsset> models;
	GMOwnedPtr<GMSkeleton> skeleton;
};

class GMModels : public GMObject
{
	GM_DECLARE_PRIVATE(GMModels)
	GM_DECLARE_PROPERTY(Models, models, Vector<GMAsset>)

public:
	void push_back(GMModelAsset model);
	void swap(GMModels* models);

public:
	inline bool isEmpty() GM_NOEXCEPT
	{
		D(d);
		return d->models.empty();
	}

	inline GMSkeleton* getSkeleton() GM_NOEXCEPT
	{
		D(d);
		return d->skeleton.get();
	}

	void setSkeleton(AUTORELEASE GMSkeleton* skeleton)
	{
		D(d);
		d->skeleton.reset(skeleton);
	}

	GMModel* operator[](GMsize_t i)
	{
		D(d);
		return d->models[i].getModel();
	}
};

typedef Vector<GMVertex> GMVertices;
typedef Vector<GMuint> GMIndices;

GM_PRIVATE_OBJECT(GMMesh)
{
	GMVertices vertices;
	GMIndices indices;
};

//! 表示一份网格数据。
/*!
  网格数据可能仅仅是模型中的一段数据。
*/
class GMMesh : public GMObject
{
	GM_DECLARE_PRIVATE(GMMesh)

public:
	GMMesh(GMModel* parent);

public:
	//! 计算网格的切线空间。
	/*!
	  切线空间的计算和拓扑模式有关。<BR>
	  如果拓扑模式是GMTopologyMode::TriangleStrip，前3个顶点将相互作为相邻顶点来计算，第4个及以后的顶点将取前2个顶点为相邻顶点。<BR>
	  如果拓扑模式是GMTopologyMode::Triangles，将采取3个顶点为一组相邻顶点来进行计算。<BR>
	  如果拓扑模式是GMTopologyMode::Lines，不会计算切线空间。
	  \param topologyMode 网格拓扑模式。
	*/
	void calculateTangentSpace(GMTopologyMode topologyMode);
	void clear();
	void vertex(const GMVertex& vertex);
	void index(GMuint index);
	void invalidateTangentSpace();
	void swap(GMVertices& vertex);
	void swap(GMIndices& indices);

public:
	const GMVertices& vertices()
	{
		D(d);
		return d->vertices;
	}

	const GMIndices& indices()
	{
		D(d);
		return d->indices;
	}

private:
	enum
	{
		InvalidTangentSpace = 0,
	};
};

END_NS
#endif