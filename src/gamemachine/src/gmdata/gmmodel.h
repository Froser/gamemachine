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
	GMMat4 finalTransformation = Zero<GMMat4>(); //!< 所有的计算结果将会放到这里来
};

GM_PRIVATE_CLASS(GMNode);
GM_DECLARE_POINTER(GMNode);

class GMNode
{
	GM_DECLARE_PRIVATE(GMNode)
	GM_DISABLE_COPY_ASSIGN(GMNode)

	GM_DECLARE_PROPERTY(GMString, Name)
	GM_DECLARE_PROPERTY(GMNodePtr, Parent)
	GM_DECLARE_PROPERTY(Vector<GMNodePtr>, Children)
	GM_DECLARE_PROPERTY(Vector<GMuint32>, ModelIndices)
	GM_DECLARE_PROPERTY(GMMat4, TransformToParent)
	GM_DECLARE_PROPERTY(GMMat4, GlobalTransform)

public:
	GMNode();
	~GMNode();
};

GM_PRIVATE_CLASS(GMSkeletalBones);
class GMSkeletalBones
{
public:
	typedef Map<GMString, GMsize_t> BoneNameIndexMap_t;

	GM_DECLARE_PRIVATE(GMSkeletalBones)
	GM_DECLARE_PROPERTY(AlignedVector<GMSkeletalBone>, Bones)
	GM_DECLARE_PROPERTY(BoneNameIndexMap_t, BoneNameIndexMap)

public:
	GMSkeletalBones();
	GMSkeletalBones(const GMSkeletalBones&);
	GMSkeletalBones(GMSkeletalBones&&) GM_NOEXCEPT;
	GMSkeletalBones& operator=(const GMSkeletalBones& rhs);
	GMSkeletalBones& operator=(GMSkeletalBones&& rhs) GM_NOEXCEPT;
};

GM_PRIVATE_CLASS(GMSkeleton);
class GMSkeleton
{
	GM_DECLARE_PRIVATE(GMSkeleton)
	GM_DECLARE_PROPERTY(GMSkeletalBones, Bones)

public:
	enum
	{
		BonesPerVertex = 4
	};

	GMSkeleton();
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

GM_ALIGNED_STRUCT(GMNodeAnimationNode)
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
	AlignedVector<GMNodeAnimationNode> nodes;
};

GM_PRIVATE_CLASS(GMSkeletalAnimations);
class GMSkeletalAnimations
{
	GM_DECLARE_PRIVATE(GMSkeletalAnimations)
	GM_DISABLE_ASSIGN(GMSkeletalAnimations)
	GM_DECLARE_PROPERTY(AlignedVector<GMNodeAnimation>, Animations)

public:
	GMSkeletalAnimations();

public:
	GMNodeAnimation* getAnimation(GMsize_t index) GM_NOEXCEPT;
	GMsize_t getAnimationCount();
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

class GMModelBuffer;
GM_PRIVATE_CLASS(GMModelDataProxy);
class GM_EXPORT GMModelDataProxy : public GMObject, public IQueriable
{
	GM_DECLARE_PRIVATE(GMModelDataProxy)

public:
	GMModelDataProxy(const IRenderContext* context, GMModel* obj);
	~GMModelDataProxy();

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
	GMModel* getModel();

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

GM_PRIVATE_CLASS(GMModelBuffer);
//! 用来管理GMModelBuffer生命周期的类，包含引用计数功能。
class GM_EXPORT GMModelBuffer
{
	GM_DECLARE_PRIVATE(GMModelBuffer)
	GM_DISABLE_COPY_ASSIGN(GMModelBuffer)

	GMModelBuffer();
	~GMModelBuffer();

	void dispose();
	void setData(const GMModelBufferData& bufferData);
	const GMModelBufferData& getMeshBuffer();
	void addRef();
	void releaseRef();
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

GM_PRIVATE_CLASS(GMModel);
class GM_EXPORT GMModel : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMModel)
	GM_DISABLE_COPY_ASSIGN(GMModel)

public:
	typedef GMfloat DataType;

public:
	GMModel();
	GMModel(GMModelAsset parentAsset);
	~GMModel();

	GM_DECLARE_PROPERTY(GMTopologyMode, PrimitiveTopologyMode);
	GM_DECLARE_PROPERTY(GMModelType, Type);
	GM_DECLARE_PROPERTY(GMShader, Shader);
	GM_DECLARE_PROPERTY(GMsize_t, VerticesCount);
	GM_DECLARE_PROPERTY(GMModelDrawMode, DrawMode);
	GM_DECLARE_PROPERTY(GMRenderTechniqueID, TechniqueId);
	GM_DECLARE_PROPERTY(Vector<GMNode*>, Nodes)
	GM_DECLARE_PROPERTY(AlignedVector<GMMat4>, BoneTransformations)

public:
	void setModelDataProxy(AUTORELEASE GMModelDataProxy* modelDataProxy);

	GMModelDataProxy* getModelDataProxy() GM_NOEXCEPT;

	GMParts& getParts() GM_NOEXCEPT;

	//! 表示此模型是否需要被GMModelDataProxy将顶点数据传输到显卡。
	/*!
	  如果一个模型第一次建立顶点数据，则需要将这些数据传输到显卡。<br/>
	  然而，如果此模型如果与其他模型共享一份顶点数据，那么此模型不需要传输顶点数据到显卡，因为数据已经存在。
	  \sa GMModelDataProxy()
	*/
	bool isNeedTransfer() GM_NOEXCEPT;

	//! 表示此模型不再需要将顶点数据传输到显卡了。
	/*!
	  当使用了已经传输过的顶点数据，或者顶点数据传输完成时调用此方法。
	*/
	void doNotTransferAnymore() GM_NOEXCEPT;

	// 绘制方式
	void setUsageHint(GMUsageHint hint) GM_NOEXCEPT;

	GMUsageHint getUsageHint() GM_NOEXCEPT;
	GMModel* getParentModel() GM_NOEXCEPT;

	GMSkeleton* getSkeleton() GM_NOEXCEPT;
	void setSkeleton(AUTORELEASE GMSkeleton* skeleton);
	void setModelBuffer(AUTORELEASE GMModelBuffer* mb);
	GMModelBuffer* getModelBuffer();
	void releaseModelBuffer();
	void addPart(GMPart* part);
};

enum class GMAnimationType
{
	NoAnimation,
	SkeletalAnimation,
	AffineAnimation,
};

GM_PRIVATE_CLASS(GMScene);
class GM_EXPORT GMScene : public GMObject
{
	GM_DECLARE_PRIVATE(GMScene)
	GM_DECLARE_PROPERTY(Vector<GMAsset>, Models)
	GM_DECLARE_PROPERTY(GMAnimationType, AnimationType)

public:
	enum
	{
		MaxBoneCount = 128,
	};

	static GMSceneAsset createSceneFromSingleModel(GMModelAsset modelAsset);

public:
	GMScene();
	void addModelAsset(GMModelAsset model);
	void swap(GMScene* scene);
	bool isEmpty() GM_NOEXCEPT;
	GMSkeletalAnimations* getAnimations() GM_NOEXCEPT;
	void setAnimations(AUTORELEASE GMSkeletalAnimations* animations);
	void setRootNode(GMNode* root);
	bool hasAnimation() GM_NOEXCEPT;
	GMNode* getRootNode() GM_NOEXCEPT;

public:
	GMModel* operator[](GMsize_t i);
};

typedef Vector<GMVertex> GMVertices;
typedef Vector<GMuint32> GMIndices;

GM_PRIVATE_CLASS(GMPart);
//! 表示模型中的一部分数据。
/*!
  网格数据可能仅仅是模型中的一段数据。
*/
class GM_EXPORT GMPart
{
	GM_DECLARE_PRIVATE(GMPart)
	GM_DISABLE_COPY_ASSIGN(GMPart)

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
	const GMVertices& vertices();
	const GMIndices& indices();

private:
	enum
	{
		InvalidTangentSpace = 0,
	};
};

END_NS
#endif