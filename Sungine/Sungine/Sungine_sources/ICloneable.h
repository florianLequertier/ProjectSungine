#pragma once

#include <memory>


////////////////////////////////
// Interface for clonable object. 
// Clone methode allows things like : 
// Titi* titi = new Toto();
// Toto* toto = titi->clone();
// IClonableWithName is the same as IClonable, except that it allow the cloned object to have a constructor taking a string.
////////////////////////////////

template<typename T>
class ICloneable
{
public:
	virtual T* clone() const = 0;
	virtual std::shared_ptr<T> cloneShared() const = 0;
};

#define CLONABLE_IMPL(BaseType, Type)\
public:\
	virtual BaseType* clone() const override\
	{\
		auto cloned = new Type();\
		return cloned;\
	}\
	virtual std::shared_ptr<BaseType> cloneShared() const override\
	{\
		auto cloned = std::make_shared<Type>();\
		return cloned;\
	}\
private:

template<typename T>
class ICloneableWithName
{
public:
	virtual T* clone(const std::string& name) = 0;
	virtual std::shared_ptr<T> cloneShared(const std::string& name) = 0;
};

#define CLONABLE_WITH_NAME_IMPL(BaseType, Type)\
	virtual BaseType* clone(const std::string& name) override\
	{\
		return new Type(name);\
	}\
	virtual std::shared_ptr<BaseType> cloneShared(const std::string& name) override\
	{\
		return std::make_shared<Type>(name);\
	}
	