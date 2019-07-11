#include "stdafx.h"
#include "helper.h"

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
