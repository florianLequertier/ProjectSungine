#pragma once

#include <string>

#include "Reflexion.h"
#include "IDGenerator.h"

class Object
{
protected:
	ID m_objectId;

public:
	Object()
	{
		m_objectId = IDGenerator<Object>::instance().lockID();
	}

	virtual ~Object()
	{
		IDGenerator<Object>::instance().freeID(m_objectId);
	}

	virtual int getClassId() const = 0;
	virtual std::string getClassName() const = 0;
	virtual bool isA(const Object& other) const = 0;
	virtual bool isA(int otherClassId) const = 0;
	virtual bool inheritFrom(const Object& other) const = 0;
	virtual bool inheritFrom(int otherClassId) const = 0;

	template<typename T>
	bool inheritFrom() const
	{
		int otherClassId = Reflexion<T>::instance().getClassId();
		return inheritFrom(otherClassId);
	}

	template<typename T>
	bool isA(int otherClassId) const
	{
		int otherClassId = Reflexion<T>::instance().getClassId();
		return isA(otherClassId);
	}

	const ID& getObjectID() const
	{
		return m_objectId;
	}
};