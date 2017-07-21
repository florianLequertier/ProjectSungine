#pragma once

#include <string>

#include "Metadatas.h"
#include "IDGenerator.h"

class Object
{
	CLASS((Object),
	((PRIVATE)
		(ID, m_objectId)
	)
	)

public:
	Object()
	{
		m_objectId = IDGenerator<Object>::instance().lockID();
	}

	virtual ~Object()
	{
		IDGenerator<Object>::instance().freeID(m_objectId);
	}

	template<typename OtherObjectClass>
	bool isA() const
	{
		return getDescriptor().isA<OtherObjectClass>();
	}

	template<typename OtherObjectClass>
	bool isA(const OtherObjectClass& otherObject) const
	{
		return getDescriptor().isA(otherObject.getDescriptor());
	}

	int getClassId() const
	{
		return getDescriptor().getClassId();
	}

	const std::string& getClassName() const
	{
		return getDescriptor().getClassName();
	}

	template<typename ClassType>
	static int getStaticClassId()
	{
		return ObjectDescriptor<ClassType>::getInstance().getClassId();
	}

	static int getClassIdFromName(const std::string& className)
	{
		return BaseObjectDescriptor::getClassIdFromName(className);
	}

	void saveJSON(cereal::JSONOutputArchive& archive) const
	{
		getDescriptor().saveObjectInstanceJSON(this, archive);
	}

	void loadJSON(cereal::JSONInputArchive& archive)
	{
		getDescriptor().loadObjectInstanceJSON(this, archive);
	}

	virtual void OnBeforeObjectSaved()
	{
		// Nothing by default
	}
	virtual void OnAfterObjectLoaded()
	{
		// Nothing by default
	}

	virtual void drawInInspector()
	{
		getDescriptor().drawInInspector(this);
	}

	virtual void drawInInspector(const std::vector<void*>& objectInstances)
	{
		getDescriptor().drawInInspector(objectInstances);
	}

	void print()
	{
		getDescriptor().printObjectInstance(this);
	}

	const ID& getObjectID() const
	{
		return m_objectId;
	}
};

REGISTER_CLASS(Object)
