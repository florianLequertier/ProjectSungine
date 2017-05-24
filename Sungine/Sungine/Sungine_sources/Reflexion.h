#pragma once

#include <string>
#include <vector>

#include "ISingleton.h"

class ReflexionBase
{
public:
	static int classCount;
};

template<typename T, typename U>
class ReflexionCounters
{
public:
	static int count;
};

template<typename T>
class Reflexion : public ReflexionBase, public ISingleton<Reflexion<T>>
{
	friend T;

public :
	static bool initialized;
	static int parentCount;

public:
	std::vector<int> inheritedClassIds;
	int classId;
	std::string className;

	template<typename U>
	inline int addInheritFrom() 
	{ 
		inheritedClassIds.push_back(Reflexion<U>::instance().getClassId());
		inheritedClassIds.insert(inheritedClassIds.end(), Reflexion<U>::instance().getInheritanceIds().begin(), Reflexion<U>::instance().getInheritanceIds().end());
		std::cout << "inheritance detected" << std::endl;
		return inheritedClassIds.size(); 
	}
	//inline bool addInheritedClassId(int id) { inheritedClassIds.push_back(id); std::cout << "inheritance detected : " << id << std::endl; return true; }
	inline bool initClassId() 
	{ 
		classId = ReflexionBase::classCount;
		std::cout << "class Id = "<< classId << std::endl;
		if (!initialized) 
			ReflexionBase::classCount++; 
		return true; 
	}
	inline bool initClassName(std::string name) 
	{ 
		className = name;
		std::cout << "class name = " << className << std::endl;
		return true;
	}

public:
	Reflexion(){}
	inline int getClassId() { return classId; }
	inline std::string getClassName() { return className; }
	inline bool isA(int otherClassId) { return classId == otherClassId; }
	inline bool inheritFrom(int otherClassId) { return std::find(inheritedClassIds.begin(), inheritedClassIds.end(), otherClassId) != inheritedClassIds.end(); }
	inline const std::vector<int>& getInheritanceIds() const { return inheritedClassIds; }

	SINGLETON_IMPL(Reflexion)
};

#define REFLEXION_HEADER(Type)\
	public:\
	inline static int staticClassId()\
	{\
		return Reflexion<Type>::instance().getClassId();\
	}\
	inline virtual int getClassId() const override\
	{\
		return Reflexion<Type>::instance().getClassId();\
	}\
	inline virtual std::string getClassName() const override\
	{\
		return Reflexion<Type>::instance().getClassName();\
	}\
	inline virtual bool isA(const Object& other) const override\
	{\
		return Reflexion<Type>::instance().isA(other.getClassId());\
	}\
	inline virtual bool inheritFrom(const Object& other) const override\
	{\
		return Reflexion<Type>::instance().inheritFrom(other.getClassId());\
	}\
	inline virtual bool isA(int otherClassId) const override\
	{\
		return Reflexion<Type>::instance().isA(otherClassId);\
	}\
	inline virtual bool inheritFrom(int otherClassId) const override\
	{\
		return Reflexion<Type>::instance().inheritFrom(otherClassId);\
	}\
	template<typename ParentType>\
	bool inheritFrom() const\
	{\
		int otherClassId = Reflexion<ParentType>::instance().getClassId();\
		return inheritFrom(otherClassId);\
	}\
	template<typename ParentType>\
	bool isA(int otherClassId) const\
	{\
		int otherClassId = Reflexion<ParentType>::instance().getClassId();\
		return isA(otherClassId);\
	}\
	private:

#define REFLEXION_CPP(Type)\
	bool Reflexion<Type>::initialized = Reflexion<Type>::instance().initClassName(#Type) && Reflexion<Type>::instance().initClassId();

#define REFLEXION_InheritFrom(Type, ParentType)\
	int ReflexionCounters<Type, ParentType>::count = Reflexion<Type>::instance().addInheritFrom<ParentType>();