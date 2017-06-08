#ifndef __MODELREADER_H__
#define __MODELREADER_H__
#include "common.h"
#include "foundation/linearmath.h"
BEGIN_NS

class Object;
struct GMBuffer;
struct GMModelLoadSettings;
struct IModelReader
{
	virtual ~IModelReader() {}
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT Object** object) = 0;
	virtual bool test(const GMBuffer& buffer) = 0;
};

class GMGamePackage;
struct GMModelLoadSettings
{
	GMGamePackage& gamePackage;
	const linear_math::Vector3& extents;
	const linear_math::Vector3& position;
	const GMString path;
	const GMString modelName;
};

class GMModelReader
{
public:
	enum ModelType
	{
		ModelType_AUTO,
		ModelType_Begin,
		ModelType_Obj = ModelType_Begin,
		ModelType_End,
	};

public:
	static bool load(const GMModelLoadSettings& settings, OUT Object** object);
	static bool load(const GMModelLoadSettings& settings, ModelType type, OUT Object** object);
	static IModelReader* getReader(ModelType type);

private:
	static ModelType test(const GMBuffer& buffer);
};

END_NS
#endif
