#pragma once

#include "Materials.h"
#include "IDrawable.h"

class IRenderableComponent
{
public:
	virtual const IDrawable& getDrawable(int drawableIndex) const = 0;
	virtual const MaterialInstance& getDrawableMaterial(int drawableIndex) const = 0;
	virtual const int getDrawableCount() const = 0;
	virtual Component* getAsComponent() = 0;
};