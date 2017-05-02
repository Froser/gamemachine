#ifndef __MODELREADER_H__
#define __MODELREADER_H__
#include "common.h"
#include "utilities/vmath.h"
BEGIN_NS

class Object;
struct GamePackageBuffer;
struct ModelLoadSettings;
struct IModelReader
{
	virtual bool load(const ModelLoadSettings& settings, GamePackageBuffer& buffer, OUT Object** object) = 0;
	virtual bool test(const GamePackageBuffer& buffer) = 0;
};

enum ModelType
{
	ModelType_AUTO,
	ModelType_Begin,
	ModelType_Obj = ModelType_Begin,
	ModelType_End,
};

class GamePackage;
struct ModelLoadSettings
{
	GamePackage& gamePackage;
	const vmath::vec3& extents;
	const vmath::vec3& position;
	const char* path;
};

class ModelReader
{
public:
	static bool load(const ModelLoadSettings& settings, OUT Object** object);
	static bool load(const ModelLoadSettings& settings, ModelType type, OUT Object** object);
	static IModelReader* getReader(ModelType type);

private:
	static ModelType test(const GamePackageBuffer& buffer);
};

END_NS
#endif