#pragma once

#include "ISerializable.h"

class IResourcePtr : public ISerializable
{
public:
	virtual void reset() = 0;
	virtual bool isValid() const = 0;
};