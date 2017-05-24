#pragma once

#include "TestMetaData01.h"

class Toto : public Titi
{
	REFLEXION_HEADER(Toto);
};

REFLEXION_CPP(Toto);
REFLEXION_InheritFrom(Toto, Titi);

void testMetaData();