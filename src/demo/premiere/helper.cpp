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
