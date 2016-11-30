#ifndef __OBJSTRUCT_H__
#define __OBJSTRUCT_H__
#include "common.h"
#include <map>

BEGIN_NS

class Image;
struct TextureInfo
{
	Image* texture;
	GMuint id;
};

typedef std::map<std::string, TextureInfo> TextureMap;

struct IObjectCallback
{
	virtual ~IObjectCallback() {};
	virtual void draw(GMuint id) = 0;
	virtual void dispose(GMuint id, const TextureMap& textureMap) = 0;
};

class GLObjectCallback : public IObjectCallback
{
public:
	void draw(GMuint id);
	void dispose(GMuint id, const TextureMap& textureMap);
};

class Object
{
public:
	Object();
	Object(GMuint id, const TextureMap& textureMap);
	~Object();
	Object& operator =(Object& obj);

public:
	void draw();

public:
	GMuint m_objectId;
	TextureMap m_textureMap;
	IObjectCallback* m_pCallback;
};

END_NS
#endif