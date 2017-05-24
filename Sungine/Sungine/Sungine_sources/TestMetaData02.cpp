#include <iostream>

#include "TestMetaData01.h"
#include "TestMetaData02.h"


void testMetaData()
{
	Toto toto;
	Tutu* tutu = new Tutu();
	Titi titi;
	Titi* totoAsTiti = new Toto();

	std::cout << "Toto className = " << toto.getClassName() << std::endl;
	std::cout << "Toto id = " << toto.getClassId() << std::endl;

	std::cout << "Titi className = " << titi.getClassName() << std::endl;
	std::cout << "Titi id = " << titi.getClassId() << std::endl;

	std::cout << "Tutu className = " << tutu->getClassName() << std::endl;
	std::cout << "Tutu id = " << tutu->getClassId() << std::endl;

	std::cout << "TotoAsTiti className = " << totoAsTiti->getClassName() << std::endl;
	std::cout << "TotoAsTiti id = " << totoAsTiti->getClassId() << std::endl;

	std::cout << "TotoAsTiti inherit from titi = " << totoAsTiti->inheritFrom(titi) << std::endl;
	std::cout << "tutu inherit from titi = " << tutu->inheritFrom(titi) << std::endl;
	std::cout << "TotoAsTiti inherit from TutuParent01 = " << tutu->inheritFrom<TutuParent01>() << std::endl;
	std::cout << "tutu inherit from from TutuInterface = " << tutu->inheritFrom<TutuInterface>() << std::endl;

	TutuParent01 tutuParent;
	std::cout << "TotoAsTiti inherit from TutuParent01 = " << tutu->inheritFrom(tutuParent) << std::endl;
}