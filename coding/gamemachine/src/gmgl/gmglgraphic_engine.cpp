#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "gmglobjectpainter.h"
#include "gmengine/elements/character.h"

/*
struct __shadowSourcePred
{
	bool operator() (GameLight* light)
	{
		return light->getShadowSource();
	}
};
*/

GMGLGraphicEngine::GMGLGraphicEngine()
{
	D(d);
	d.world = nullptr;
	d.settings = nullptr;
	d.shadowMapping.reset(new GMGLShadowMapping(*this));
}

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	for (auto iter = d.allShaders.begin(); iter != d.allShaders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}

	for (auto iter = d.allRenders.begin(); iter != d.allRenders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}
}

void GMGLGraphicEngine::initialize(GameWorld* world)
{
	D(d);
	d.world = world;
	d.shadowMapping->init();
}

void GMGLGraphicEngine::newFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GMGLGraphicEngine::drawObjects(DrawingList& drawingList)
{
	applyGraphicSettings();
	/*
	GameLight* shadowSourceLight = getShadowSourceLight();
	if (shadowSourceLight)
	{
		d.shadowMapping->beginDrawDepthBuffer(shadowSourceLight);
		drawObjectsOnce(drawingList, !!shadowSourceLight);
		d.shadowMapping->endDrawDepthBuffer();
	}
	*/
	drawObjectsOnce(drawingList, false);
}

void GMGLGraphicEngine::applyGraphicSettings()
{
	glLineWidth(0);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CW);
}

void GMGLGraphicEngine::drawObjectsOnce(DrawingList& drawingList, bool shadowOn)
{
	D(d);
	bool shadowMapping = d.shadowMapping->hasBegun();
	int i = 0;
	GMGLShaders* lastShaders = nullptr;
	for (auto iter = drawingList.begin(); iter != drawingList.end(); iter++)
	{
		DrawingItem& item = *iter;
		Object* coreObj = item.gameObject->getObject();
		coreObj->getPainter()->draw();

		/*
		Object* coreObj = item.gameObject->getObject();
		BEGIN_FOREACH_OBJ(coreObj, coreChildObj)
		{
			ChildObject::ObjectType type = coreChildObj->getType();
			GMGLShaders& shaders = shadowMapping ? d.shadowMapping->getShaders() : *getShaders(type);

			shaders.useProgram();
			GMGL::uniformMatrix4(shaders, item.trans, GMSHADER_MODEL_MATRIX);
			GMGLObjectPainter* painter = static_cast<GMGLObjectPainter*>(coreObj->getPainter());

			if (!shadowMapping)
			{
				if (type != ChildObject::Glyph)
				{
					if (lastShaders != &shaders)
					{
						setEyeViewport(shadowOn, shaders);
						lastShaders = &shaders;
					}
				}
				shadowTexture(shadowOn, shaders);
				painter->setWorld(d.world);
			}
			else
			{
				painter->setWorld(nullptr);
			}

			painter->draw();
		}
		END_FOREACH_OBJ
		*/
	}
}

GMGLShadowMapping* GMGLGraphicEngine::getShadowMapping()
{
	D(d);
	return d.shadowMapping;
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	D(d);
	updateMatrices(lookAt);

	BEGIN_ENUM(i, ChildObject::ObjectTypeBegin, ChildObject::ObjectTypeEnd)
	{
		if (i == ChildObject::Glyph)
			continue;

		GMGLShaders* shaders = getShaders(i);
		if (!shaders)
			continue;
		shaders->useProgram();
		GMGL::lookAt(d.viewMatrix, *shaders, GMSHADER_VIEW_MATRIX);
		GMGL::cameraPosition(lookAt, *shaders, GMSHADER_VIEW_POSITION);
	}
	END_ENUM
}

void GMGLGraphicEngine::updateMatrices(const CameraLookAt& lookAt)
{
	D(d);
	Character* character = getWorld()->getMajorCharacter();

	//TODO 
	d.projectionMatrix = character->getFrustum().getPerspective();
	d.viewMatrix = getViewMatrix(lookAt);

	character->getFrustum().updateViewMatrix(d.viewMatrix, d.projectionMatrix);
	character->getFrustum().update();
}

GameWorld* GMGLGraphicEngine::getWorld()
{
	D(d);
	return d.world;
}

void GMGLGraphicEngine::registerShader(ChildObject::ObjectType objectType, AUTORELEASE GMGLShaders* shaders)
{
	D(d);
	d.allShaders[objectType] = shaders;
}

GMGLShaders* GMGLGraphicEngine::getShaders(ChildObject::ObjectType objectType)
{
	D(d);
	if (d.allShaders.find(objectType) == d.allShaders.end())
		return nullptr;
	
	return d.allShaders[objectType];
}

void GMGLGraphicEngine::registerRender(ChildObject::ObjectType objectType, AUTORELEASE IRender* render)
{
	D(d);
	d.allRenders[objectType] = render;
}

IRender* GMGLGraphicEngine::getRender(ChildObject::ObjectType objectType)
{
	D(d);
	if (d.allRenders.find(objectType) == d.allRenders.end())
		return nullptr;

	return d.allRenders[objectType];
}

ResourceContainer* GMGLGraphicEngine::getResourceContainer()
{
	D(d);
	return &d.resourceContainer;
}

GraphicSettings* GMGLGraphicEngine::getGraphicSettings()
{
	D(d);
	ASSERT(d.settings);
	return d.settings;
}

void GMGLGraphicEngine::setGraphicSettings(GraphicSettings* settings)
{
	D(d);
	d.settings = settings;
}