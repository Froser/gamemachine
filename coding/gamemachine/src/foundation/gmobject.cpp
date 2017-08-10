﻿#include "stdafx.h"
#include "gmobject.h"
#include <utility>

GMObject::GMObject(GMObject&& obj) noexcept
{
	*this = std::move(obj);
}

GMObject& GMObject::operator=(GMObject&& obj) noexcept
{
	if (this != &obj)
	{
		swap(*this, obj);
		obj.dataWrapper()->m_data = nullptr;
	}
	return *this;
}

void GMObject::swap(GMObject& another)
{
	swap(*this, another);
}

void GMObject::swap(GMObject& a, GMObject& b)
{
	GMObjectPrivateWrapper<GMObject>* wrapperA = a.dataWrapper(),
		*wrapperB = b.dataWrapper();
	if (!wrapperA || !wrapperB)
		return;
	wrapperA->swap(wrapperB);
}