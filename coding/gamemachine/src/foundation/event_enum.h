#ifndef __EVENT_ENUM
#define __EVENT_ENUM
BEGIN_NS

#define GM_SET_PROPERTY_ENUM(name) GMEVENT_SET_##name

enum
{
	// GMLight
	GM_SET_PROPERTY_ENUM(Type),

	// GMTextureFrames
	GM_SET_PROPERTY_ENUM(FrameCount),
	GM_SET_PROPERTY_ENUM(AnimationMs),
	GM_SET_PROPERTY_ENUM(MagFilter),
	GM_SET_PROPERTY_ENUM(MinFilter),
	GM_SET_PROPERTY_ENUM(WrapS),
	GM_SET_PROPERTY_ENUM(WrapT),

	// Shader
	GM_SET_PROPERTY_ENUM(SurfaceFlag),
	GM_SET_PROPERTY_ENUM(Cull),
	GM_SET_PROPERTY_ENUM(FrontFace),
	GM_SET_PROPERTY_ENUM(BlendFactorSource),
	GM_SET_PROPERTY_ENUM(BlendFactorDest),
	GM_SET_PROPERTY_ENUM(Blend),
	GM_SET_PROPERTY_ENUM(Nodraw),
	GM_SET_PROPERTY_ENUM(NoDepthTest),
	GM_SET_PROPERTY_ENUM(Texture),
	GM_SET_PROPERTY_ENUM(LineWidth),
	GM_SET_PROPERTY_ENUM(LineColor),
	GM_SET_PROPERTY_ENUM(DrawBorder),
	GM_SET_PROPERTY_ENUM(Material),

	// GMParticles
	GM_SET_PROPERTY_ENUM(CurrentLife),
	GM_SET_PROPERTY_ENUM(MaxLife),
	GM_SET_PROPERTY_ENUM(Color),
	GM_SET_PROPERTY_ENUM(Transform),

	// GMControlGameObject
	GM_SET_PROPERTY_ENUM(Stretch),
};

END_NS
#endif