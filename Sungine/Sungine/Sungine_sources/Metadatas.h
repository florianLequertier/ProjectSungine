#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>

#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/control/if.hpp>

#include "Fields.h"

template<typename T>
void atomicPrint(const T& object)
{
	//TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// METADATA SYSTEM
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//// The property descriptor is able to retrieve the instance member and can perform actions on this member (saving, printing, ...)
struct IPropertyDescriptor
{
	virtual void saveJSON(const void* object, cereal::JSONOutputArchive& archive) = 0;
	virtual void loadJSON(void* object, cereal::JSONInputArchive& archive) = 0;
	virtual void print(const void* object) = 0;
	virtual void drawInInspector(void* object) = 0;
	virtual void drawInInspector(const std::vector<void*>& objectInstances) = 0;
	virtual const std::string& getName() const = 0;
};

template<typename Class, typename MemberType>
class PropertyDescriptor : public IPropertyDescriptor
{
public:
	MemberType  Class::*pointerToMember;
	MetasFilter<MemberType> metasFilter;
	std::string name;

	PropertyDescriptor(MemberType Class::* ptrToMember, const std::string& propertyName)
		: pointerToMember(ptrToMember)
		, name(propertyName)
	{}

	PropertyDescriptor(MemberType Class::* ptrToMember, const std::string& propertyName, const std::string& metas)
		: metasFiltermetas()
		, pointerToMember(ptrToMember)
		, name(propertyName)
	{}

	const std::string& getName() const override
	{
		return name;
	}

	MemberType& getInstanceMember(void* objectInstance)
	{
		return *static_cast<Class*>(objectInstance).*pointerToMember;
	}

	const MemberType& getInstanceMember(const void* objectInstance)
	{
		return *static_cast<const Class*>(objectInstance).*pointerToMember;
	}

	virtual void saveJSON(const void* objectInstance, cereal::JSONOutputArchive& archive) override
	{
		archive(getInstanceMember(objectInstance));
	}

	virtual void loadJSON(void* objectInstance, cereal::JSONInputArchive& archive) override
	{
		archive(getInstanceMember(objectInstance));
	}

	virtual void print(const void* objectInstance) override
	{
		atomicPrint(getInstanceMember(objectInstance));
	}

	virtual void drawInInspector(void* objectInstance) override
	{
		Field::PropertyField(this, std::vector<void*>(1, objectInstance), metasFilter);
	}

	virtual void drawInInspector(const std::vector<void*>& objectInstances) override
	{
		std::vector<MemberType*> properties(objectInstances.size());
		int index = 0;
		for (auto& objectInstance : objectInstances)
		{
			properties[index++] = &getInstanceMember(objectInstance);
		}

		Field::PropertyField(properties, metasFilter);
	}
};

template<typename Class, typename MemberType>
void doStuffWithPropertyInDescriptor(PropertyDescriptor<Class, MemberType>& propertyDescriptor)
{
	MemberType& member = propertyDescriptor.getInstanceMember();

	//doStuffWith with member
}

template<typename Class, typename MemberType>
void printPropertyInDescriptor(PropertyDescriptor<Class, MemberType>& propertyDescriptor)
{
	MemberType& member = propertyDescriptor.getInstanceMember();

	//doStuffWith with member
	std::cout << member << std::endl;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//// The object descriptor store informations about a specific class : 
//// - It stores its propertyDescriptors in order to handle actions on all properties (saving, printing, ...)
//// - It handle pointers to propertyDecriptors wich match with the class parents. This way we can performs action not only on class properties but also on parent class properties (i.e performing polymorphism)
//// The IObjectDescriptorAction is the interface which allow us to perform these actions.
class IObjectDescriptor
{
public:
	virtual void saveObjectInstanceJSON(const void* objectInstance, cereal::JSONOutputArchive& archive) = 0;
	virtual void loadObjectInstanceJSON(void* objectInstance, cereal::JSONInputArchive& archive) = 0;
	virtual void printObjectInstance(const void* objectInstance) = 0;
	virtual void drawInInspector(void* objectInstance) = 0;
	virtual void drawInInspector(const std::vector<void*>& objectInstances) = 0;
	virtual void drawInInspectorExcept(void* objectInstance, const std::vector<int>& ignoredIndex) = 0;
	virtual void drawInInspectorExcept(const std::vector<void*>& objectInstances, const std::vector<int>& ignoredIndex) = 0;
};

class BaseObjectDescriptor : public IObjectDescriptor
{
protected:
	static int ClassCount;
	static std::unordered_map<std::string, int> classNameToId;

	int classId;
	std::string className;
	std::vector<BaseObjectDescriptor*> parentClassDescriptors;
	std::vector<IPropertyDescriptor*> propertyDescriptors;
	std::unordered_map<std::string, int> propertyDescriptorsMapping;

public:

	BaseObjectDescriptor()
	{
		classId = ClassCount;
		ClassCount++;
	}

	int getClassId() const
	{
		return classId;
	}

	const std::string& getClassName() const
	{
		return className;
	}

	static int getClassIdFromName(const std::string& className)
	{
		auto found = classNameToId.find(className);
		if (found != classNameToId.end())
			return found->second;
	}

	template<typename OtherObjectClass>
	bool isA()
	{
		if (classId == ObjectDescriptor<OtherObjectClass>::getClassId())
			return true;
		else
		{
			for (parentClassDescriptor : parentClassDescriptors)
			{
				if (parentClassDescriptor->isA<OtherObjectClass>())
					return true;
			}
			return false;
		}
	}

	template<typename OtherObjectDescriptorClass>
	bool isA(const OtherObjectDescriptorClass& otherObjectDescriptor) const
	{
		if (isA_recur(otherObjectDescriptor))
			return true;
		else
			return otherObjectDescriptor.isA(*this);
	}

	template<typename OtherObjectDescriptorClass>
	bool isA_recur(const OtherObjectDescriptorClass& otherObjectDescriptor) const
	{
		if (classId == otherObjectDescriptor.getClassId())
			return true;
		else
		{
			for (auto parentClassDescriptor : parentClassDescriptors)
			{
				if (parentClassDescriptor->isA(otherObjectDescriptor))
					return true;
			}
			return false;
		}
	}

	std::vector<IPropertyDescriptor*>::iterator getPropertyDescriptorsBegin()
	{
		return propertyDescriptors.begin();
	}

	std::vector<IPropertyDescriptor*>::iterator getPropertyDescriptorsEnd()
	{
		return propertyDescriptors.end();
	}

	int getPropertyIndex(const std::string& propertyName)
	{
		auto found = propertyDescriptorsMapping.find(propertyName);
		if (found != propertyDescriptorsMapping.end())
			return found->second;
		else
			return -1;
	}

	IPropertyDescriptor* getPropertyDescriptor(const std::string& propertyName)
	{
		int propertyIndex = getPropertyIndex(propertyName);
		if (propertyIndex != -1)
		{
			return propertyDescriptors[propertyIndex];
		}
		else
		{
			return nullptr;
		}
	}

	IPropertyDescriptor* getPropertyDescriptor(int propertyIndex)
	{
		if (propertyIndex >= 0 && propertyIndex < propertyDescriptors.size())
		{
			return propertyDescriptors[propertyIndex];
		}
		else
		{
			return nullptr;
		}
	}
};

template<typename ObjectClass>
class ObjectDescriptor final : public BaseObjectDescriptor
{
public:
	static bool isRegistered;

public:
	/////////////// Registering
	template<typename MemberClass>
	static void registerProperty(MemberClass ObjectClass::* pointerToMember , const std::string& propertyName)
	{
		getInstance().propertyDescriptors.push_back(new PropertyDescriptor<ObjectClass, MemberClass>(pointerToMember, propertyName));
		getInstance().propertyDescriptorsMapping[memberName] = getInstance().propertyDescriptors.size() - 1;
	}

	template<typename MemberClass>
	static void registerProperty(MemberClass ObjectClass::* pointerToMember, const std::string& propertyName, const std::string& metas)
	{
		getInstance().propertyDescriptors.push_back(new PropertyDescriptor<ObjectClass, MemberClass>(pointerToMember, propertyName, metas));
		getInstance().propertyDescriptorsMapping[memberName] = getInstance().propertyDescriptors.size() - 1;
	}

	template<typename ParentClass>
	static void registerParentClass()
	{
		getInstance().parentClassDescriptors.push_back(&ObjectDescriptor<ParentClass>::getInstance());
	}

	static void registerClassName(const std::string& name)
	{
		getInstance().className = name;
		classNameToId[name] = classId;
	}
	/////////////////////////////

	/////////////// Singleton implementation
	static ObjectDescriptor<ObjectClass>& getInstance()
	{
		static ObjectDescriptor<ObjectClass>* instance = new ObjectDescriptor<ObjectClass>();
		return *instance;
	}

	ObjectDescriptor()
	{}

	ObjectDescriptor(const ObjectDescriptor& other) = delete;
	ObjectDescriptor& operator=(const ObjectDescriptor& other) = delete;
	/////////////////////////////

	void saveObjectInstanceJSON(const void* objectInstance, cereal::JSONOutputArchive& archive) override
	{
		// We take parent classes into account
		for (auto parentClass : parentClassDescriptors)
		{
			parentClass->saveObjectInstanceJSON(objectInstance, archive);
		}

		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			propertyDescriptor->saveJSON(objectInstance, archive);
		}
	}

	void loadObjectInstanceJSON(void* objectInstance, cereal::JSONInputArchive& archive) override
	{
		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->loadObjectInstanceJSON(objectInstance, archive);
		}

		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			propertyDescriptor->loadJSON(objectInstance, archive);
		}
	}

	void printObjectInstance(const void* objectInstance) override
	{
		std::cout << "Object class ID :" << classId << std::endl;

		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->printObjectInstance(objectInstance);
		}

		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			propertyDescriptor->print(objectInstance);
		}
	}

	virtual void drawInInspector(void* objectInstance) override
	{
		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->drawInInspector(objectInstance);
		}

		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			propertyDescriptor->drawInInspector(objectInstance);
		}
	}

	virtual void drawInInspector(const std::vector<void*>& objectInstances) override
	{
		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->drawInInspector(objectInstances);
		}

		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			propertyDescriptor->drawInInspector(objectInstances);
		}
	}
	
	virtual void drawInInspectorExcept(void* objectInstance, const std::vector<int>& ignoredIndex) override
	{
		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->drawInInspectorExcept(objectInstance);
		}

		int index = 0;
		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			if (std::find(ignoredIndex.begin(); ignoredIndex.end(), index) == ignoredIndex.end())
				propertyDescriptor->drawInInspector(objectInstance);
			index++;
		}
	}

	virtual void drawInInspectorExcept(const std::vector<void*>& objectInstances, const std::vector<int>& ignoredIndex) override
	{
		// We take parent classes into account
		for (auto parentClass : getInstance().parentClassDescriptors)
		{
			parentClass->drawInInspectorExcept(objectInstances);
		}

		int index = 0;
		for (auto propertyDescriptor : getInstance().propertyDescriptors)
		{
			if (std::find(ignoredIndex.begin(); ignoredIndex.end(), index) == ignoredIndex.end())
				propertyDescriptor->drawInInspector(objectInstances);
			index++;
		}
	}
	/////////////////////////////
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Macros to register the class and this infos (parent classes, properties, ...)
#define REGISTER_CLASS(Class) template<> bool ObjectDescriptor<Class>::isRegistered = Class::registerProperties();
#define OBJECT_CLASS(Class, ClassRegisterContent) public: friend class ObjectDescriptor<Class>; virtual BaseObjectDescriptor& getDescriptor() const{ return ObjectDescriptor<Class>::getInstance(); } static bool registerProperties() { ClassRegisterContent; return true;} private:


//#define PROPERTY(PROP) ( ##PROP, 1)
#define PUBLIC 0
#define PRIVATE 1
#define PROTECTED 2

//#define PROPERTY(...) (__VA_ARGS__, 1 )
//#define PROPERTY_PUBLIC(...) (__VA_ARGS__, 0)
//#define PROPERTY_PRIVATE(...) (__VA_ARGS__, 1)
//#define PROPERTY_PROTECTED(...) (__VA_ARGS__, 2)
//#define DEFINE_PROP_PUBLIC(elem) public : BOOST_PP_TUPLE_ELEM(0, elem) BOOST_PP_TUPLE_ELEM(1, elem); private :
//#define DEFINE_PROP_PRIVATE(elem) private : BOOST_PP_TUPLE_ELEM(0, elem) BOOST_PP_TUPLE_ELEM(1, elem); private :
//#define DEFINE_PROP_PROTECTED(elem) protected : BOOST_PP_TUPLE_ELEM(0, elem) BOOST_PP_TUPLE_ELEM(1, elem); private :
//#define DEFINE_PROP_PRIV_OR_PRO(elem) BOOST_PP_IF( BOOST_PP_SUB( BOOST_PP_TUPLE_ELEM(3, elem), 1) , PROPERTY_PROTECTED(elem), DEFINE_PROP_PRIVATE(elem))
//#define DEFINE_PROP(r, data, elem) BOOST_PP_IF( BOOST_PP_TUPLE_ELEM(3, elem) , DEFINE_PROP_PRIV_OR_PRO(elem), DEFINE_PROP_PUBLIC(elem))

#define GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_0(...) \
     ((__VA_ARGS__)) GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_1

#define GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_1(...) \
     ((__VA_ARGS__)) GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_0

#define GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_0_END
#define GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_1_END

// Double the parentheses of a Boost.PP sequence
// I.e. (a, b)(c, d) becomes ((a, b))((c, d))
#define GLK_PP_SEQ_DOUBLE_PARENS(seq) \
    BOOST_PP_CAT(GLK_PP_DETAIL_SEQ_DOUBLE_PARENS_0 seq, _END)

#define PP_SEQ_FOR_EACH_R_ID() BOOST_PP_SEQ_FOR_EACH_R 
#define PP_DEFER(x) x BOOST_PP_EMPTY()
#define BOOST_PP_IF_ID() BOOST_PP_IF
#define DEFINE_PROP_PRIV_OR_PRO_ID() DEFINE_PROP_PRIV_OR_PRO
#define DEFINE_PROP_PUBLIC_ID() DEFINE_PROP_PUBLIC
#define BOOST_PP_SUB_ID() BOOST_PP_SUB

#define DEFINE_PROP__(r, data, elem) BOOST_PP_TUPLE_ELEM(0, elem) BOOST_PP_TUPLE_ELEM(1, elem);

#define DEFINE_PROP_PUBLIC(r, elem) public : PP_DEFER(PP_SEQ_FOR_EACH_R_ID)()(r, DEFINE_PROP__,, GLK_PP_SEQ_DOUBLE_PARENS(elem)) //private :
#define DEFINE_PROP_PRIVATE(r, elem) private : PP_DEFER(PP_SEQ_FOR_EACH_R_ID)()(r, DEFINE_PROP__,, GLK_PP_SEQ_DOUBLE_PARENS(elem)) //private :
#define DEFINE_PROP_PROTECTED(r, elem) protected : PP_DEFER(PP_SEQ_FOR_EACH_R_ID)()(r, DEFINE_PROP__,, GLK_PP_SEQ_DOUBLE_PARENS(elem)) //private :

#define DEFINE_PROP_PRIV_OR_PRO(r, scope, elem) PP_DEFER(BOOST_PP_IF_ID)()( BOOST_PP_SUB( scope, 1) , DEFINE_PROP_PROTECTED(r, elem), DEFINE_PROP_PRIVATE(r, elem))

// public, (prop01) (prop02) ...
#define DEFINE_PROP_(r, scope, elem) PP_DEFER(BOOST_PP_IF_ID)()( scope, DEFINE_PROP_PRIV_OR_PRO(r, scope, elem), DEFINE_PROP_PUBLIC(r, elem))

// (public) (prop01) (prop02) ...)) -> REGISTER_PROP_( public, (prop01) (prop02) ,... )
#define DEFINE_PROP(r, data, elem) DEFINE_PROP_(r, BOOST_PP_SEQ_ELEM(0, elem), BOOST_PP_SEQ_POP_FRONT(elem))

// ((public) (prop01) (prop02) ...) ((private) (prop01) (prop02) ...) ...) -> foreach((public) (prop01) (prop02) ...)->REGISTER_PROP)
#define DEFINE_PROPS( seq ) protected: BOOST_PP_SEQ_FOR_EACH(DEFINE_PROP,_, seq) private:

//class, type, name, meta -> Object<class>::registerProperty<type>(&class::name, meta)
#define REGISTER_PROP____(Class, Type, Name, Metas) ObjectDescriptor<Class>::registerProperty<Type>(&Class::Name, #Name, #Metas);
#define REGISTER_PROP____BIS(Class, Type, Name) ObjectDescriptor<Class>::registerProperty<Type>(&Class::Name, #Name);

//class, prop01 -> REGISTER_PROP___(class, prop01[0], prop01[1], prop01[2])
#define REGISTER_PROP___(r, Class, elem) REGISTER_PROP____(Class, BOOST_PP_TUPLE_ELEM(0, elem), BOOST_PP_TUPLE_ELEM(1, elem), BOOST_PP_TUPLE_ELEM(2, elem))
#define REGISTER_PROP___BIS(r, Class, elem) REGISTER_PROP____BIS(Class, BOOST_PP_TUPLE_ELEM(0, elem), BOOST_PP_TUPLE_ELEM(1, elem))

#define REGISTER_PROP__(r, Class, elem) BOOST_PP_IF(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(elem), 2), REGISTER_PROP___(r, Class, elem), REGISTER_PROP___BIS(r, Class, elem))

//class, public, (prop01) (prop02) ...) -> foreach( (prop01) (prop02) ...) -> REGISTER_PROP__)
#define REGISTER_PROP_(r, Class, Scope, elem) PP_DEFER(PP_SEQ_FOR_EACH_R_ID)()(r, REGISTER_PROP__, Class, GLK_PP_SEQ_DOUBLE_PARENS(elem))

//class, (public) (prop01) (prop02) ... -> REGISTER_PROP_( class, public, (prop01) (prop02) ... )
#define REGISTER_PROP(r, Class, elem) REGISTER_PROP_(r, Class, BOOST_PP_SEQ_ELEM(0, elem), BOOST_PP_SEQ_POP_FRONT(elem))

#define REGISTER_PARENT(r, Class, parent_class) ObjectDescriptor<Class>::registerParentClass<parent_class>();

#define REGISTER_PARENTS(Class, seq_parents) BOOST_PP_SEQ_FOR_EACH(REGISTER_PARENT, Class, seq_parents)

//class, ((public) (prop01) (prop02) ...) ((public) (prop01) (prop02) ...) ...)) -> class / foreach((public) (prop01) (prop02) ...))->REGISTER_PROP)
#define REGISTER_PROPS(Class, Class_and_parents, seq_props)\
public:\
	static bool registerProperties() {\
		ObjectDescriptor<Class>::registerClassName(#Class);\
		BOOST_PP_IF( BOOST_PP_SUB(BOOST_PP_SEQ_SIZE(Class_and_parents), 1), REGISTER_PARENTS(Class, BOOST_PP_SEQ_POP_FRONT(Class_and_parents)), )\
		BOOST_PP_SEQ_FOR_EACH(REGISTER_PROP, Class, seq_props)\
		return true;\
	}\
private:

#define REGISTER_CLASS(Class) template<> bool ObjectDescriptor<Class>::isRegistered = Class::registerProperties();

#define CLASS_(Class, Class_and_parents, seq_props)\
	friend class ObjectDescriptor<Class>;\
	BaseObjectDescriptor& getDescriptor() const{ return ObjectDescriptor<Class>::getInstance(); }\
	REGISTER_PROPS(Class, Class_and_parents, seq_props)\
	DEFINE_PROPS(seq_props)

#define CLASS(Class_and_parents, seq_props)\
	CLASS_(BOOST_PP_SEQ_ELEM(0, BOOST_PP_TUPLE_TO_SEQ(Class_and_parents)), BOOST_PP_TUPLE_TO_SEQ(Class_and_parents), seq_props)\
private:

// For interfaces :
// You can also have ObjectDescriptor for interfaces.
// The only difference is that you cannot register parent classes or member variables.
// So you can have an interface registered as parent class : CLASS((MyClass, MyInterface) ,...)

#define INTERFACE(Class)\
	friend class ObjectDescriptor<Class>;\
	virtual BaseObjectDescriptor& getDescriptor() const{ return ObjectDescriptor<Class>::getInstance(); }\
public:\
	static bool registerProperties() {\
		ObjectDescriptor<Class>::registerClassName(#Class);\
		return true;\
	}\
private:

#define REGISTER_INTERFACE(Class) template<> bool ObjectDescriptor<Class>::isRegistered = Class::registerProperties();


// Macro usage :
//
// class ClassName{
//  ObjectClass(ClassName, 
//		[Register all properties, parent classes, ...]
//  )
// private:
//		[class members and functions ...]
// public:
//		[class function and functions ...]
// }
// REGISTER_CLASS(ClassName)
//
// Macros create a getDescriptor() function in you objects so to call meta functions on the object all you need is to call :
// Foo foo;
// foo.getDecriptor().doMetaStuff(&foo);
//
// It must be convenient to have a base class which 
// expose function like :
// doStuff() {
// this->getDecriptor().doMetaStuff(this);
// }
// This is the role of the Object class of the next example part : 
