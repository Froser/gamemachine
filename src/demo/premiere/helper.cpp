#include "stdafx.h"
#include "helper.h"
#include <extensions/bsp/gmbspfactory.h>
#include <gmfontmetrics.h>

GMRect Helper::getMiddleRectOfWindow(const GMRect& rc, IWindow* window)
{
	GMRect wrc = window->getRenderRect();
	GMRect result = {
		(wrc.width - rc.width) / 2,
		(wrc.height - rc.height) / 2,
		rc.width,
		rc.height
	};
	return result;
}

FontColorAnimationKeyframe::FontColorAnimationKeyframe(GMTextGameObject* textObject, const GMVec4& color, GMfloat timePoint)
	: GMAnimationKeyframe(timePoint)
	, m_textObject(textObject)
	, m_timeStart(0)
	, m_targetColor(color)
{
	m_defaultColor = textObject->getColor();

	m_colorMap[textObject] = m_defaultColor;
}

void FontColorAnimationKeyframe::reset(IDestroyObject* object)
{
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	textObject->setColor(m_defaultColor);
}

void FontColorAnimationKeyframe::beginFrame(IDestroyObject* object, GMfloat timeStart)
{
	m_timeStart = timeStart;
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	m_colorMap[textObject] = textObject->getColor();
}

void FontColorAnimationKeyframe::endFrame(IDestroyObject* object)
{
}

void FontColorAnimationKeyframe::update(IDestroyObject* object, GMfloat time)
{
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	GMfloat percentage = (time - m_timeStart) / (getTime() - m_timeStart);
	textObject->setColor(Lerp(m_colorMap[textObject], m_targetColor, percentage));
}

GameObjectColorKeyframe::GameObjectColorKeyframe(Map<GMModel*, GMVertices>& cache, const Array<GMfloat, 4>& color, GMfloat timePoint)
	: GMAnimationKeyframe(timePoint)
	, m_color(color)
	, m_cache(cache)
	, m_timeStart(0)
{

}

void GameObjectColorKeyframe::reset(IDestroyObject* object)
{
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->foreachModel([this](GMModel* m) {
		update(m, m_cache[m], &m_colorMap[m][0]);
	});
}

void GameObjectColorKeyframe::beginFrame(IDestroyObject* object, GMfloat timeStart)
{
	m_timeStart = timeStart;

	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->foreachModel([this](GMModel* m) {
		auto iter = m_cache.find(m);
		if (iter != m_cache.end() && !iter->second.empty())
		{
			m_colorMap[m] = iter->second[0].color;
		}
	});
}

void GameObjectColorKeyframe::endFrame(IDestroyObject* object)
{
	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	gameObj->foreachModel([this](GMModel* m) {
		update(m, m_cache[m], &m_color[0]);
	});
}

void GameObjectColorKeyframe::update(IDestroyObject* object, GMfloat time)
{
	GMfloat percentage = (time - m_timeStart) / (getTime() - m_timeStart);
	if (percentage > 1.f)
		percentage = 1.f;

	GMGameObject* gameObj = gm_cast<GMGameObject*>(object);
	const auto& functor = getFunctors();
	gameObj->foreachModel([this, &functor, percentage](GMModel* m) {
		const auto& rgbaStartArray = m_colorMap[m];
		GMVec4 rgbaStart (rgbaStartArray[0], rgbaStartArray[1], rgbaStartArray[2], rgbaStartArray[3]);
		GMVec4 rgbaEnd (m_color[0], m_color[1], m_color[2], m_color[3]);
		GMVec4 color = functor.vec4Functor->interpolate(rgbaStart, rgbaEnd, percentage);
		update(m, m_cache[m], ValuePointer(color));
	});
}

void GameObjectColorKeyframe::update(GMModel* model, const GMVertices& vertices, const GMfloat color[4])
{
	GMVertices& cache = m_cache[model];
	GM_ASSERT(model->getVerticesCount() == cache.size());

	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
	for (GMsize_t i = 0; i < cache.size(); ++i)
	{
		cache[i].color = { color[0], color[1], color[2], color[3] };
	}

	memcpy_s(proxy->getBuffer(), sizeof(GMVertex) * model->getVerticesCount(), cache.data(), sizeof(GMVertex) * cache.size());
	proxy->endUpdateBuffer();
}

BSPGameObject::BSPGameObject(const IRenderContext* context)
{
	setContext(context);
}

void BSPGameObject::load(const GMBuffer& buffer)
{
	BSPGameWorld* world = new BSPGameWorld(getContext());
	world->loadBSP(buffer);

	if (world)
	{
		if (auto phyw = world->getPhysicsWorld())
		{
			phyw->setEnabled(false);
		}

		m_world.reset(world);
		m_lookAtCache = m_world->getSprite()->getCamera().getLookAt();
	}
}

bool BSPGameObject::isValid()
{
	return !!m_world.get();
}

void BSPGameObject::draw()
{
	if (m_world)
		m_world->renderScene();
}

void BSPGameObject::update(GMDuration dt)
{
	if (m_world)
	{
		m_world->updateGameWorld(dt);
	}
}

void BSPGameObject::onAppendingObjectToWorld()
{
	GMCamera& camera = getContext()->getEngine()->getCamera();
	camera.lookAt(m_lookAtCache);
	GMGameObject::onAppendingObjectToWorld();
}

constexpr GMfloat BOTTOM_POS = -2.f;

ScreenObject::ScreenObject(const IRenderContext* context)
	: GMGameObject()
	, m_context(context)
	, m_spacing(20)
	, m_baseline(0)
	, m_maxWidth(0)
	, m_playing(false)
	, m_rollingSpeed(2)
	, m_positionY(BOTTOM_POS)
{
	m_world.reset(new GMGameWorld(context));
}

void ScreenObject::draw()
{
	m_world->renderScene();
}

void ScreenObject::setSpacing(GMint32 s)
{
	m_spacing = s;
}

void ScreenObject::setSpeed(GMint32 speed)
{
	GMRect renderRC = m_context->getWindow()->getRenderRect();
	m_rollingSpeed = (speed / (2.f * renderRC.height));
}

void ScreenObject::addText(const GMString& text, const TextOptions& options)
{
	GM.invokeInMainThread([this, text, options]() {
		GMFontMetrics::FontAttributes attributes;
		attributes.font = options.font;
		attributes.fontSize = options.fontSize;

		GMRect renderRC = m_context->getWindow()->getRenderRect();
		GMFontMetrics fontMetrices(attributes, m_context);
		GMRect metrices = fontMetrices.boundingRect(text);
		metrices.y += m_baseline;
		metrices.x = 0;
		metrices.width = renderRC.width;

		GMTextGameObject* obj = new GMTextGameObject(renderRC);
		obj->setCenter(true);
		obj->setGeometry(metrices);
		obj->setText(text);
		obj->setFont(options.font);
		obj->setFontSize(options.fontSize);
		obj->setColorType(GMTextColorType::Plain);
		GMVec4 color;
		color.setFloat4(options.fontColor);
		obj->setColor(color);
		m_world->addObjectAndInit(obj);
		m_world->addToRenderList(obj);
		m_baseline += metrices.height + m_spacing;
		m_maxWidth = Max(m_maxWidth, metrices.width);
		m_objects.push_back(obj);
	});
}

void ScreenObject::addImage(const GMBuffer& buffer)
{
	GM.invokeInMainThread([this, buffer]() {
		GMImage* image = nullptr;
		if (GMImageReader::load(buffer.getData(), buffer.getSize(), &image))
		{
			GMTextureAsset asset;
			GM.getFactory()->createTexture(m_context, image, asset);

			GMRect renderRC = m_context->getWindow()->getRenderRect();
			GMSprite2DGameObject* obj = new GMSprite2DGameObject(renderRC);
			obj->setTexture(asset);
			obj->setTextureSize(image->getWidth(), image->getHeight());
			GMRect textureRect = { 0, 0, image->getWidth(), image->getHeight() };
			obj->setTextureRect(textureRect);

			GMRect metrices = textureRect;
			metrices.y += m_baseline;
			metrices.x = (renderRC.width - metrices.width) * .5f;
			obj->setGeometry(metrices);

			m_baseline += metrices.height + m_spacing;
			m_world->addObjectAndInit(obj);
			m_world->addToRenderList(obj);
			m_objects.push_back(obj);
		}
		else
		{
			gm_warning(gm_dbg_wrap("Cannot open image from buffer."));
		}
	});
}

void ScreenObject::addSpacing(GMint32 spacing)
{
	GM.invokeInMainThread([this, spacing]() {
		m_baseline += spacing;
	});
}

void ScreenObject::update(GMDuration dt)
{
	if (m_playing)
	{
		m_positionY += m_rollingSpeed * dt; // 向上为正

		for (auto object : m_objects)
		{
			object->setTranslation(Translate(GMVec3(0, m_positionY, 0)));
		}
	}
}

void ScreenObject::play()
{
	m_playing = true;
}

void ScreenObject::pause()
{
	m_playing = false;
}

void ScreenObject::reset(bool)
{
	// not support. do nothing
}

void ScreenObject::onAppendingObjectToWorld()
{
	// 初始位置
	for (auto object : m_objects)
	{
		object->setTranslation(Translate(GMVec3(0, m_positionY, 0)));
	}
}
