#include "stdafx.h"
#include "gmcontrolgameobject.h"
#include "foundation/utilities/gmprimitivecreator.h"

//////////////////////////////////////////////////////////////////////////
GMControlEvent::GMControlEvent(GMControlEventType type, GMEventName eventName)
	: m_type(type)
	, m_eventName(eventName)
{
}

bool GM2DMouseDownEvent::buttonDown(Button button)
{
	if (button == GM2DMouseDownEvent::Left)
		return !!(m_state.trigger_button & GMMouseButton_Left);
	if (button == GM2DMouseDownEvent::Right)
		return !!(m_state.trigger_button & GMMouseButton_Right);
	if (button == GM2DMouseDownEvent::Middle)
		return !!(m_state.trigger_button & GMMouseButton_Middle);
	return false;
}
//////////////////////////////////////////////////////////////////////////
GMControlGameObject::GMControlGameObject(GMControlGameObject* parent)
{
	D(d);
	if (parent)
		parent->addChild(this);

	GMRect client = GM.getMainWindow()->getClientRect();
	d->clientSize = client;
}

GMControlGameObject::~GMControlGameObject()
{
	D(d);
	for (auto& child : d->children)
	{
		delete child;
	}

	if (d->stencil)
	{
		GMModel* model = d->stencil->getModel();
		GM_delete(model);
		GM_delete(d->stencil);
	}
}

void GMControlGameObject::onAppendingObjectToWorld()
{
	D(d);
	// 创建一个模板，绘制子对象的时候，子对象不能溢出此模板
	class __Cb : public IPrimitiveCreatorShaderCallback
	{
	public:
		virtual void onCreateShader(Shader& shader) override
		{
			shader.setBlend(true);
			shader.setBlendFactorDest(GMS_BlendFunc::ONE);
			shader.setBlendFactorSource(GMS_BlendFunc::ONE);
		}
	} _cb;

	GMModel* model = nullptr;
	createQuadModel(&_cb, &model);
	d->stencil = new GMGameObject(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
	d->stencil->setWorld(getWorld());
	GM.initObjectPainter(d->stencil->getModel());

	Base::onAppendingObjectToWorld();
}

void GMControlGameObject::setScaling(const linear_math::Matrix4x4& scaling)
{
	D(d);
	Base::setScaling(scaling);
	if (d->stencil)
		d->stencil->setScaling(scaling);
	for (auto& child : d->children)
	{
		child->setScaling(scaling);
	}
}

void GMControlGameObject::draw()
{
	updateAnimation();
	Base::draw();
}

void GMControlGameObject::notifyControl()
{
	D(d);
	updateUI();

	IInput* input = GM.getMainWindow()->getInputMananger();
	IMouseState& mouseState = input->getMouseState();
	GMMouseState ms = mouseState.mouseState();

	if (insideGeometry(ms.posX, ms.posY))
	{
		if (ms.trigger_button != GMMouseButton_None)
		{
			GM2DMouseDownEvent e(ms);
			event(&e);
		}
		else
		{
			if (!d->mouseHovered)
			{
				d->mouseHovered = true;
				GM2DMouseHoverEvent e(ms);
				event(&e);
			}
		}
	}
	else
	{
		if (d->mouseHovered)
		{
			d->mouseHovered = false;
			GM2DMouseLeaveEvent e(ms);
			event(&e);
		}
	}
}

void GMControlGameObject::updateAnimation()
{
	D(d);
	GMAnimationStates_t& state = d->animationStates[GMAnimationTypes::Scaling];
	GMfloat now = GM.getGameTimeSeconds();
	if (state.state == GMAnimateState::Running ||
		state.state == GMAnimateState::Reverting )
	{
		state.p += state.direction * (now - state.tick) / state.duration;
		if (state.p >= 1.f)
		{
			state.p = 1.f;
			state.state = GMAnimateState::Stopped;
		}
		else if (state.p < 0)
		{
			state.p = 0;
			state.state = GMAnimateState::Stopped;
		}

		GMfloat scaling[3];
		state.interpolation(
			state.start,
			state.end,
			state.p,
			scaling);

		linear_math::Matrix4x4 s = linear_math::scale(scaling[0], scaling[1], scaling[2]);
		setScaling(s);
	}
	state.tick = now;
}

void GMControlGameObject::animateScaleStart(const GMfloat(&end)[3], GMfloat duration, GMInterpolation interpolation)
{
	D(d);
	GMAnimationStates_t& state = d->animationStates[GMAnimationTypes::Scaling];
	GMfloat now = GM.getGameTimeSeconds();
	state.interpolation = interpolation;
	auto& scalingMatrix = getScaling();
	state.start[0] = scalingMatrix[0][0];
	state.start[1] = scalingMatrix[1][1];
	state.start[2] = scalingMatrix[2][2];
	state.duration = duration;
	state.tick = now;
	state.p = 0;
	state.direction = 1;
	memcpy_s(state.end, sizeof(end), end, sizeof(end));
	state.state = GMAnimateState::Running;
}

void GMControlGameObject::animateScaleEnd()
{
	D(d);
	GMAnimationStates_t& state = d->animationStates[GMAnimationTypes::Scaling];
	state.direction = -1;
	state.state = GMAnimateState::Reverting;
}

void GMControlGameObject::event(GMControlEvent* e)
{
	D(d);
	emitEvent(e->getEventName());
}

bool GMControlGameObject::insideGeometry(GMint x, GMint y)
{
	D(d);
	return d->parent ?
		d->parent->insideGeometry(x, y) && GM_in_rect(d->geometry, x, y) :
		GM_in_rect(d->geometry, x, y);
}

void GMControlGameObject::updateUI()
{
	D(d);
	switch (GM.peekMessage().msgType)
	{
	case GameMachineMessageType::WindowSizeChanged:
		GMRect nowClient = GM.getMainWindow()->getClientRect();
		GMfloat scaleX = (GMfloat)nowClient.width / d->clientSize.width,
			scaleY = (GMfloat)nowClient.height / d->clientSize.height;

		GM_ASSERT(scaleX != 0 && scaleY != 0);

		if (getStretch())
		{
			d->geometry.x *= scaleX;
			d->geometry.y *= scaleY;
			d->geometry.width *= scaleX;
			d->geometry.height *= scaleY;
		}
		else
		{
			// 调整大小，防止拉伸
			auto& mat = getScaling();
			GMfloat scaling[] = { 1.f / scaleX * mat[0][0], 1.f / scaleY * mat[1][1], mat[2][2] };
			setScaling(linear_math::scale({ 1.f / scaleX, 1.f / scaleY, 1 }));

			// 相对于左上角位置也不能变
			auto& trans = getTranslation();
			GMRectF rect = toViewportCoord(d->geometry);
			setTranslate(linear_math::translate({ rect.x + trans[3][0], rect.y + trans[3][1], trans[3][2] }));
		}

		d->clientSize = nowClient;
		break;
	}
}

GMRectF GMControlGameObject::toViewportCoord(const GMRect& in)
{
	GMRect client = GM.getMainWindow()->getClientRect();
	GMRectF out = {
		in.x * 2.f / client.width - 1.f,
		1.f - in.y * 2.f / client.height,
		(GMfloat)in.width / client.width,
		(GMfloat)in.height / client.height
	};
	return out;
}

void GMControlGameObject::addChild(GMControlGameObject* child)
{
	D(d);
	child->setParent(this);
	d->children.push_back(child);
}

void GMControlGameObject::createQuadModel(IPrimitiveCreatorShaderCallback* callback, OUT GMModel** model)
{
	D(d);
	GM_ASSERT(model);

	GMRectF coord = toViewportCoord(d->geometry);
	GMfloat extents[3] = {
		coord.width,
		coord.height,
		1.f,
	};
	GMfloat pos[3] = { coord.x, coord.y, 0 };
	GMPrimitiveCreator::createQuad(extents, pos, model, callback, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft);
}

GMAnimateState GMControlGameObject::getAnimationState(GMAnimationTypes::Types type)
{
	D(d);
	return d->animationStates[type].state;
}