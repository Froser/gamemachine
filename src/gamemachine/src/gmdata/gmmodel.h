#ifndef __GMMODEL_H__
#define __GMMODEL_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <linearmath.h>
#include <gmimage.h>
#include <gmshader.h>
#include <atomic>
#include <gmrendertechnique.h>

struct ID3D11Buffer;

BEGIN_NS

class GMAnimationEvaluator;
struct GMSceneAnimatorNode;
struct GMNodeAnimation;

struct GMSkeletonWeight
{
	GMuint32 vertexId;
	GMfloat weight;
};

GM_ALIGNED_STRUCT(GMSkeletalBone)
{
	GMString name;
	GMMat4 offsetMatrix; //!< 从模型空间到绑定姿势的变换
	GMModel* targetModel = nullptr;
	Vector<GMSkeletonWeight> weights;
	GMMat4 finalTransformation; //!< 所有的计算结果将会放到这里来
};

GM_PRIVATE_OBJECT(GMNode)
{
	GMString name;
	GMNode* parent = nullptr;
	Vector<GMNode*> children;
	GMMat4 transformToParent;
	GMMat4 globalTransform;
	Vector<GMuint32> modelIndices;
};

GM_ALIGNED_16(class) GMNode
{
	GM_DECLARE_PRIVATE_NGO(GMNode)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Name, name)
	GM_DECLARE_PROPERTY(Parent, parent)
	GM_DECLARE_PROPERTY(Children, children)
	GM_DECLARE_PROPERTY(ModelIndices, modelIndices)
	GM_DECLARE_PROPERTY(TransformToParent, transformToParent)
	GM_DECLARE_PROPERTY(GlobalTransform, globalTransform)

public:
	~GMNode()
	{
		for (auto& child : getChildren())
		{
			GM_delete(child);
		}
	}
};

GM_PRIVATE_OBJECT(GMSkeletalBones)
{
	AlignedVector<GMSkeletalBone> bones;
	Map<GMString, GMsize_t> boneNameIndexMap;
};

class GMSkeletalBones
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalBones)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Bones, bones)
	GM_DECLARE_PROPERTY(BoneNameIndexMap, boneNameIndexMap)
};

GM_PRIVATE_OBJECT(GMSkeleton)
{
	GMSkeletalBones bones;
};

class GMSkeleton
{
	GM_DECLARE_PRIVATE_NGO(GMSkeleton)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Bones, bones)

public:
	enum
	{
		BonesPerVertex = 4
	};
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
struct GMNodeAnimationKeyframe
{
	GMNodeAnimationKeyframe(GMDuration t, T&& v)
		: time(t)
		, value(v)
	{
	}

	GMDuration time;
	T value;
};

GM_ALIGNED_STRUCT(GMAnimationNode)
{
	GMString name;
	AlignedVector<GMNodeAnimationKeyframe<GMVec3>> positions;
	AlignedVector<GMNodeAnimationKeyframe<GMVec3>> scalings;
	AlignedVector<GMNodeAnimationKeyframe<GMQuat>> rotations;
};

GM_ALIGNED_STRUCT(GMNodeAnimation)
{
	GMfloat frameRate = 25;
	GMDuration duration;
	GMString name;
	AlignedVector<GMAnimationNode> nodes;
};

GM_PRIVATE_OBJECT(GMSkeletalAnimations)
{
	AlignedVector<GMNodeAnimation> animations;
};

GM_ALIGNED_16(class) GMSkeletalAnimations
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalAnimations)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Animations, animations)

public:
	inline GMNodeAnimation* getAnimation(GMsize_t index) GM_NOEXCEPT
	{
		D(d);
		return &d->animations[index];
	}

	inline GMsize_t getAnimationCount()
	{
		D(d);
		return d->animations.size();
	}
};

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
		BoneIDsDimension = GMSkeleton::BonesPerVertex,
		WeightsDimension = GMSkeleton::BonesPerVertex,
	};

	Array<GMfloat, PositionDimension> positions;
	Array<GMfloat, NormalDimension> normals;
	Array<GMfloat, TexcoordDimension> texcoords;
	Array<GMfloat, TangentDimension> tangents;
	Array<GMfloat, BitangentDimension> bitangents;
	Array<GMfloat, LightmapDimension> lightmaps;
	Array<GMfloat, ColorDimension> color;
	Array<GMint32, BoneIDsDimension> boneIds;
	Array<GMfloat, WeightsDimension> weights;
};

typedef Vector<GMPart*> GMParts;

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

class GMPart;
class GMModelBuffer;
class GM_EXPORT GMModelDataProxy : public GMObject, public IQueriable
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
	void packIndices(Vector<GMuint32>& indices);
	void prepareParentModel();
};

enum class GMUsageHint
{
	StaticDraw,
	DynamicDraw,
};

class GMPart;
struct GMModelBufferData
{
	union
	{
		struct //OpenGL
		{
			GMuint32 arrayId;
			GMuint32 vertexBufferId;
			GMuint32 indexBufferId;
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
	GMAtomic<GMint32> ref;
	GMModelDataProxy* modelDataProxy = nullptr;
};

//! 用来管理GMModelBuffer生命周期的类，包含引用计数功能。
class GM_EXPORT GMModelBuffer : public GMObject
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
	TriangleStrip,
	Triangles,
	Lines,
};

enum class GMModelDrawMode
{
	Vertex,
	Index,
};

class GMModel;
GM_PRIVATE_OBJECT(GMModel)
{
	GMUsageHint hint = GMUsageHint::StaticDraw;
	GMParts parts;
	GMOwnedPtr<GMModelDataProxy> modelDataProxy;
	GMShader shader;
	GMModelBuffer* modelBuffer = nullptr;
	GMModelDrawMode drawMode = GMModelDrawMode::Vertex;
	GMModelType type = GMModelType::Model3D;
	GMTopologyMode mode = GMTopologyMode::Triangles;
	GMsize_t verticesCount = 0;
	bool needTransfer = true;
	GMRenderTechniqueID techniqueId = 0;
	GMModelAsset parentAsset;
	GMOwnedPtr<GMSkeleton> skeleton;
	Vector<GMNode*> nodes;
	// 骨骼变换矩阵，对于无骨骼的model，使用首个元素表示变换。
	AlignedVector<GMMat4> boneTransformations;
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
	BoneIds,
	Weights,

	// ---
	EndOfVertexDataType
};

#define gmVertexIndex(i) ((GMuint32)i)

GM_ALIGNED_16(class) GM_EXPORT GMModel : public IDestroyObject
{
	GM_DECLARE_PRIVATE_NGO(GMModel)
	GM_DECLARE_ALIGNED_ALLOCATOR()

public:
	typedef GMfloat DataType;

public:
	GMModel();
	GMModel(GMModelAsset parentAsset);
	~GMModel();

	GM_DECLARE_PROPERTY(PrimitiveTopologyMode, mode);
	GM_DECLARE_PROPERTY(Type, type);
	GM_DECLARE_PROPERTY(Shader, shader);
	GM_DECLARE_PROPERTY(VerticesCount, verticesCount);
	GM_DECLARE_PROPERTY(DrawMode, drawMode);
	GM_DECLARE_PROPERTY(TechniqueId, techniqueId);
	GM_DECLARE_PROPERTY(Nodes, nodes)
	GM_DECLARE_PROPERTY(BoneTransformations, boneTransformations)

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

	inline GMParts& getParts() GM_NOEXCEPT
	{
		D(d);
		return d->parts;
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
	inline void setUsageHint(GMUsageHint hint) GM_NOEXCEPT
	{
		D(d);
		d->hint = hint;
	}

	inline GMUsageHint getUsageHint() GM_NOEXCEPT
	{
		D(d);
		return d->hint;
	}

	inline GMModel* getParentModel() GM_NOEXCEPT
	{
		D(d);
		return d->parentAsset.getModel();
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

	void setModelBuffer(AUTORELEASE GMModelBuffer* mb);
	GMModelBuffer* getModelBuffer();
	void releaseModelBuffer();
	void addPart(GMPart* part);
};

#define GM_DEFINE_VERTEX_DATA(name) \
	Vector<GMModel::DataType> name;

#define GM_DEFINE_VERTEX_PROPERTY(name) \
	inline auto& name() { D(d); return d->name; }

enum class GMAnimationType
{
	NoAnimation,
	SkeletalAnimation,
	AffineAnimation,
};

GM_PRIVATE_OBJECT(GMScene)
{
	Vector<GMAsset> models;
	GMOwnedPtr<GMSkeletalAnimations> animations;
	GMOwnedPtr<GMNode> root;
	GMAnimationType animationType = GMAnimationType::NoAnimation;
};

class GM_EXPORT GMScene : public GMObject
{
	GM_DECLARE_PRIVATE(GMScene)
	GM_DECLARE_PROPERTY(Models, models)
	GM_DECLARE_PROPERTY(AnimationType, animationType)

public:
	enum
	{
		MaxBoneCount = 128,
	};

	static GMSceneAsset createSceneFromSingleModel(GMModelAsset modelAsset);

public:
	void addModelAsset(GMModelAsset model);
	void swap(GMScene* scene);

public:
	inline bool isEmpty() GM_NOEXCEPT
	{
		D(d);
		return d->models.empty();
	}

	inline GMSkeletalAnimations* getAnimations() GM_NOEXCEPT
	{
		D(d);
		return d->animations.get();
	}

	void setAnimations(AUTORELEASE GMSkeletalAnimations* animations)
	{
		D(d);
		GM_ASSERT(d->animationType != GMAnimationType::NoAnimation);
		d->animations.reset(animations);
	}

	void setRootNode(GMNode* root)
	{
		D(d);
		d->root.reset(root);
	}

	bool hasAnimation() GM_NOEXCEPT
	{
		D(d);
		return !!d->animations;
	}

	GMNode* getRootNode()
	{
		D(d);
		return d->root.get();
	}

	GMModel* operator[](GMsize_t i)
	{
		D(d);
		return d->models[i].getModel();
	}
};

typedef Vector<GMVertex> GMVertices;
typedef Vector<GMuint32> GMIndices;

GM_PRIVATE_OBJECT(GMPart)
{
	GMVertices vertices;
	GMIndices indices;
};

//! 表示模型中的一部分数据。
/*!
  网格数据可能仅仅是模型中的一段数据。
*/
class GM_EXPORT GMPart : public GMObject
{
	GM_DECLARE_PRIVATE(GMPart)

public:
	GMPart(GMModel* parent);

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
	bool calculateNormals(GMModelDrawMode drawMode, GMTopologyMode topologyMode, GMS_FrontFace frontFace);
	void clear();
	void vertex(const GMVertex& vertex);
	void index(GMuint32 index);
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