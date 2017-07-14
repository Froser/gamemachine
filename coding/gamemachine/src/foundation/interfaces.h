#ifndef __INTERFACES_H__
#define __INTERFACES_H__
BEGIN_NS

// 前置声明
class GameMachine;
class GMGameWorld;
class GMCamera;
class ResourceContainer;
class GMGameObject;
class DrawingList;
class GMGamePackage;
class GMImage;
class Object;
class GMGamePackage;
class GMGlyphManager;
class GMUIWindow;
class Shader;
class GMObjectPainter;
class GMMesh;
class GMTextureFrames;
class GMTexture;
struct ISoundPlayer;
struct IGamePackageHandler;
struct GraphicSettings;
struct CameraLookAt;

namespace linear_math
{
	class Matrix4x4;
}

enum class GameMachineEvent
{
	FrameStart,
	FrameEnd,
	Simulate,
	Render,
	Activate,
	Deactivate,
	Terminate,
};

GM_INTERFACE(IGameHandler)
{
	virtual void init() = 0;
	virtual void start() = 0;
	virtual void event(GameMachineEvent evt) = 0;
	virtual bool isWindowActivate() = 0;
};

GM_INTERFACE(ITexture)
{
	virtual void drawTexture(GMTextureFrames* frames) = 0;
};


enum class GMLightType
{
	AMBIENT = 0,
	SPECULAR,

	// 以上类型数目
	COUNT,
};

GM_PRIVATE_OBJECT(GMLight)
{
	GMLightType type = GMLightType::AMBIENT;
	GMfloat lightPosition[3];
	GMfloat lightColor[3];
};

class GMLight : public GMObject
{
	DECLARE_PRIVATE(GMLight)

public:
	GM_DECLARE_PROPERTY(Type, type, GMLightType);

	GMLight() = default;

	GMLight(const GMLight& light)
	{
		*this = light;
	}

	GMLight(GMLightType type)
	{
		D(d);
		d->type = type;
	}

	inline GMLight& operator=(const GMLight& rhs)
	{
		D(d);
		D_OF(rhs_d, &rhs);
		*d = *rhs_d;
		setLightColor(rhs_d->lightColor);
		setLightPosition(rhs_d->lightPosition);
		return *this;
	}

	void setLightColor(GMfloat light[3])
	{
		D(d);
		d->lightColor[0] = light[0];
		d->lightColor[1] = light[1];
		d->lightColor[2] = light[2];
	}

	const GMfloat* getLightColor() const
	{
		D(d);
		return d->lightColor;
	}

	void setLightPosition(GMfloat light[3])
	{
		D(d);
		d->lightPosition[0] = light[0];
		d->lightPosition[1] = light[1];
		d->lightPosition[2] = light[2];
	}

	const GMfloat* getLightPosition() const
	{
		D(d);
		return d->lightPosition;
	}
};


GM_INTERFACE(IGraphicEngine)
{
	virtual void start() = 0;
	virtual void newFrame() = 0;
	virtual void setViewport(const GMRect& rect) = 0;
	virtual void drawObject(GMGameObject* obj) = 0;
	virtual void updateCameraView(const CameraLookAt& lookAt) = 0;
	virtual ResourceContainer* getResourceContainer() = 0;
	virtual void addLight(const GMLight& light) = 0;
};

enum class GMDrawMode
{
	Fill,
	Line,
};

struct IRender
{
	virtual ~IRender() {}
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform) = 0;
	virtual void beginShader(Shader& shader, GMDrawMode mode) = 0;
	virtual void endShader() = 0;
	virtual void end() = 0;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) = 0;
	virtual void activateLight(const GMLight& light, GMint lightIndex) = 0;
};


enum GamePackageType
{
	GPT_DIRECTORY,
	GPT_ZIP,
};

GM_INTERFACE(IFactory)
{
	virtual void createWindow(OUT GMUIWindow**) = 0;
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(AUTORELEASE GMImage*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, Object*, OUT GMObjectPainter**) = 0;
	virtual void createGamePackage(GMGamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
	virtual void createGlyphManager(OUT GMGlyphManager**) = 0;
};
END_NS
#endif