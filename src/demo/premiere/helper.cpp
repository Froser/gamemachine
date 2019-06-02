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

FontColorAnimationKeyframe::FontColorAnimationKeyframe(GMTextGameObject* textObject, GMVec4 color, GMfloat timePoint)
	: GMAnimationKeyframe(timePoint)
	, m_textObject(textObject)
	, m_timeStart(0)
	, m_targetColor(color)
{
	m_defaultColor = textObject->getColor();

	m_colorMap[textObject] = m_defaultColor;
}

void FontColorAnimationKeyframe::reset(GMObject* object)
{
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	textObject->setColor(m_defaultColor);
}

void FontColorAnimationKeyframe::beginFrame(GMObject* object, GMfloat timeStart)
{
	m_timeStart = timeStart;
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	m_colorMap[textObject] = textObject->getColor();
}

void FontColorAnimationKeyframe::endFrame(GMObject* object)
{
}

void FontColorAnimationKeyframe::update(GMObject* object, GMfloat time)
{
	GMTextGameObject* textObject = gm_cast<GMTextGameObject*>(object);
	GMfloat percentage = (time - m_timeStart) / (getTime() - m_timeStart);
	textObject->setColor(Lerp(m_colorMap[textObject], m_targetColor, percentage));
}
