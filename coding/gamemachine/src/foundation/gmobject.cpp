#include "stdafx.h"
#include "gmobject.h"

GMObject::~GMObject()
{

}

GMObjectPrivateWrapper<GMObject>* GMObject::dataWrapper()
{
	return nullptr;
}

void GMObject::swapData(GMObject* a, GMObject* b)
{
	GMObjectPrivateWrapper<GMObject>* wrapperA = a->dataWrapper(),
		*wrapperB = b->dataWrapper();
	if (!wrapperA || !wrapperB)
		return;
	wrapperA->swap(wrapperB);
}