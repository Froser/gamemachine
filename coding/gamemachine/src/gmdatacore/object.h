#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <vector>
#include "image.h"
#include "utilities/assert.h"
#include "utilities/autoptr.h"
#include "utilities/vmath.h"
#include "gmdatacore/texture.h"

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

enum TextureType
{
	TextureTypeUnknown = -1,
	TextureTypeShadow = 0,

	// 从Start到End表示每个对象绘制时需要清理掉的纹理
	TextureTypeResetStart,
	TextureTypeAmbient = TextureTypeResetStart,
	TextureTypeCubeMap,
	TextureTypeDiffuse,
	TextureTypeNormalMapping,
	TextureTypeResetEnd,

	// 由于反射的天空纹理存在于环境，所以不需要清理
	TextureTypeReflectionCubeMap,
};

struct TextureInfo
{
	ITexture* texture;
	ITexture* normalMapping;
	TextureType type;
	GMuint autorelease : 1;
};

enum
{
	MaxTextureCount = 6
};

struct Material
{
	GMfloat Ka[3];
	GMfloat Kd[3];
	GMfloat Ks[3];
	GMfloat Ke[3];	//对于环境（如天空）的反射系数
	GMfloat shininess;
	TextureInfo textures[MaxTextureCount];
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

	Component();
	~Component();

	Material& getMaterial()
	{
		return m_material;
	}

	void setOffset(GMvertexoffset offset)
	{
		m_offset = offset;
	}

	GMvertexoffset getOffset()
	{
		return m_offset;
	}

	GMuint getCount()
	{
		return m_verticesCount;
	}

	// 每增加一个面的时候，应该调用此函数记录这个面有多少条边
	void pushBackVertexOffset(GMuint edgesCount)
	{
		m_vertexOffsets.push_back(m_polygonEdges.empty() ?
			m_offset : m_vertexOffsets.back() + m_polygonEdges.back()
		);
		m_polygonEdges.push_back(edgesCount);
		m_polygonCount++;
	}

	GMint* getOffsetPtr()
	{
		return m_vertexOffsets.data();
	}

	GMint* getEdgeCountPtr()
	{
		return m_polygonEdges.data();
	}

	GMuint getPolygonCount()
	{
		return m_polygonCount;
	}

private:
	GMuint m_verticesCount;
	GMvertexoffset m_offset;
	Material m_material;

	// 每一个面的顶点个数
	std::vector<GMint> m_polygonEdges;
	std::vector<GMint> m_vertexOffsets;
	GMuint m_polygonCount;
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

		// 表示一个不透明的对象
		NormalObject,

		// 表示一个天空
		Sky,

		ObjectTypeEnd,
	};

	// 绘制时候的排列方式
	enum ArrangementMode
	{
		// 默认排列，按照Components，并按照一个个三角形来画
		Triangle_Fan,

		Triangle_Strip,
	};

	ChildObject();
	ChildObject(const std::string& name);
	~ChildObject();

	void clone(OUT ChildObject** childObject);

	void disposeMemory();

	void appendComponent(AUTORELEASE Component* component, GMuint verticesCount);

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

	void setVisibility(bool v)
	{
		m_visibility = v;
	}

	bool getVisibility()
	{
		return m_visibility;
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
	GMuint m_arrayId;
	GMuint m_bufferId;
	std::vector<Component*> m_components;
	ObjectType m_type;
	ArrangementMode m_mode;
	std::string m_name;
	bool m_visibility;
};

END_NS
#endif