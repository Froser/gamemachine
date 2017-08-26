#ifndef __MODELREADER_H__
#define __MODELREADER_H__
#include "common.h"
#include "foundation/linearmath.h"
BEGIN_NS

class GMModel;
struct GMBuffer;
struct GMModelLoadSettings;
struct IModelReader
{
	virtual ~IModelReader() {}
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModel** object) = 0;
	virtual bool test(const GMBuffer& buffer) = 0;
};

class GMGamePackage;
struct GMModelLoadSettings
{
	GMGamePackage& gamePackage;
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
	static bool load(const GMModelLoadSettings& settings, OUT GMModel** object);
	static bool load(const GMModelLoadSettings& settings, ModelType type, OUT GMModel** object);
	static IModelReader* getReader(ModelType type);

private:
	static ModelType test(const GMBuffer& buffer);
};

END_NS
#endif
