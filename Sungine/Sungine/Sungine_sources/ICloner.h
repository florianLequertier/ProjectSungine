#pragma once

#include <memory>

template<typename BaseType>
class ICloner
{
public:
	virtual BaseType* clone() = 0;
	virtual std::shared_ptr<BaseType> cloneShared() = 0;
};

template<typename BaseType, typename DerivedType>
class Cloner : public ICloner<BaseType>
{
public:
	virtual BaseType* clone() override { return new DerivedType(); }
	virtual std::shared_ptr<BaseType> cloneShared() override { return std::make_shared<DerivedType>(); }
};

template<typename BaseType>
class IClonerWithName
{
public:
	virtual BaseType* clone(const std::string& name) = 0;
	virtual std::shared_ptr<BaseType> cloneShared(const std::string& name) = 0;
};

template<typename BaseType, typename DerivedType>
class ClonerWithName : public IClonerWithName<BaseType>
{
public:
	virtual BaseType* clone(const std::string& name) override { return new DerivedType(name); }
	virtual std::shared_ptr<BaseType> cloneShared(const std::string& name) override { return std::make_shared<DerivedType>(name); }
};