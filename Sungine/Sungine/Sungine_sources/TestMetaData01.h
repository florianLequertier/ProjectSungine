#pragma once

#include "Object.h"
#include "Reflexion.h"

class Titi : public Object
{
	REFLEXION_HEADER(Titi)
};

REFLEXION_CPP(Titi)

class TutuParent01 : public Object
{
	REFLEXION_HEADER(TutuParent01)
};

REFLEXION_CPP(TutuParent01)

class TutuInterface
{
};

class Tutu : public TutuParent01, public TutuInterface
{
	REFLEXION_HEADER(Tutu)
};

REFLEXION_CPP(Tutu)
REFLEXION_InheritFrom(Tutu, TutuParent01)
REFLEXION_InheritFrom(Tutu, TutuInterface)