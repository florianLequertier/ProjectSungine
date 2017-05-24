#pragma once

#include "Utils.h"

#include "jsoncpp/json/json.h"
#include "ISingleton.h"
#include "IResourcePtr.h"
#include "IDGenerator.h"
#include "Resource.h"

enum ResourceType
{
	NONE,
	TEXTURE,
	CUBE_TEXTURE,
	MESH,
	SKELETAL_ANIMATION,
	MATERIAL,
	SHADER_PROGRAM,
	ANIMATION_STATE_MACHINE,
};

//static unsigned int s_resourceCount = 0;

//class ResourceManager : public ISingleton<ResourceManager>
//{ 
//	template<typename T>
//	void addResource()
//	{
//
//	}
//
//	template<typename T>
//	bool removeResource()
//	{
//
//	}
//
//public:
//	SINGLETON_IMPL(ResourceManager);
//};

template<typename T>
class ResourcePtr : public IResourcePtr
{
	template<typename U>
	friend class ResourcePtr;

private:
	T* m_rawPtr;
	ID m_resourceHashKey;
	bool m_isDefaultResource;
public:
	ResourcePtr()
		: m_isDefaultResource(true)
		, m_rawPtr(nullptr)
	{
	}

	ResourcePtr(const Resource* resource)
		: m_isDefaultResource(true)
		, m_resourceHashKey()
	{
		m_rawPtr = const_cast<T*>(static_cast<const T*>(resource));

		if (m_rawPtr != nullptr)
		{
			m_isDefaultResource = resource->getIsDefaultResource();
			m_resourceHashKey = resource->getResourceID();
			m_rawPtr->addReferenceToThis(this);
		}
	}

	ResourcePtr(const Resource& resource)
		: m_isDefaultResource(resource.getIsDefaultResource())
		, m_resourceHashKey(resource.getResourceID())
		, m_rawPtr(&resource)
	{
		m_rawPtr->addReferenceToThis(this);
	}

	ResourcePtr<T>& operator=(const Resource& resource)
	{
		if (m_rawPtr != nullptr)
			reset();

		m_isDefaultResource = resource.getIsDefaultResource();
		m_resourceHashKey = resource.getResourceID();
		m_rawPtr = &resource;

		m_rawPtr->addReferenceToThis(this);
		return *this;
	}

	ResourcePtr(const ResourcePtr<T>& other)
	{
		m_rawPtr = other.m_rawPtr;
		m_resourceHashKey = other.m_resourceHashKey;
		m_isDefaultResource = other.m_isDefaultResource;

		if(m_rawPtr != nullptr)
			m_rawPtr->addReferenceToThis(this);
	}

	ResourcePtr<T>& operator=(const ResourcePtr<T>& other)
	{
		if (m_rawPtr != nullptr)
			reset();

		m_isDefaultResource = other.m_isDefaultResource;
		m_resourceHashKey = other.m_resourceHashKey;
		m_rawPtr = const_cast<T*>(other.m_rawPtr); // TODO : REFACTOR

		if(m_rawPtr != nullptr)
			m_rawPtr->addReferenceToThis(this);
		return *this;
	}

	template<typename U>
	ResourcePtr(const ResourcePtr<U>& other)
	{
		m_rawPtr = static_cast<T*>(other.m_rawPtr);
		m_resourceHashKey = other.m_resourceHashKey;
		m_isDefaultResource = other.m_isDefaultResource;

		m_rawPtr->addReferenceToThis(this);
	}

	template<typename U>
	ResourcePtr<T>& operator=(const ResourcePtr<U>& other)
	{
		if (m_rawPtr != nullptr)
			reset();

		m_rawPtr = static_cast<T*>(other.m_rawPtr);
		m_resourceHashKey = other.m_resourceHashKey;
		m_isDefaultResource = other.m_isDefaultResource;

		if (m_rawPtr != nullptr)
			m_rawPtr->addReferenceToThis(this);
		return *this;
	}

	ResourcePtr(const Json::Value & entityRoot)
		: m_isDefaultResource(false)
		, m_rawPtr(nullptr)
	{
		load(entityRoot);

		assert(m_rawPtr != nullptr);
		m_rawPtr->addReferenceToThis(this);
	}

	~ResourcePtr()
	{
		if (m_rawPtr != nullptr)
		{
			m_rawPtr->removeReferenceToThis(this);
		}
	}

	T* operator->() const
	{
		return m_rawPtr;
	}

	T& operator*()
	{
		return *m_rawPtr;
	}

	T* get()
	{
		return m_rawPtr;
	}

	const T* get() const
	{
		return m_rawPtr;
	}

	bool operator==(const ResourcePtr<T>& other) const
	{
		return m_rawPtr == other.m_rawPtr;
	}

	bool isValid() const override
	{
		return m_rawPtr != nullptr && m_resourceHashKey.isValid();
	}

	void reset() override
	{
		if (m_rawPtr != nullptr)
			m_rawPtr->removeReferenceToThis(this);

		m_isDefaultResource = true;
		m_resourceHashKey.reset();
		m_rawPtr = nullptr;
	}

	virtual void save(Json::Value & entityRoot) const override
	{
		entityRoot["isValid"] = isValid();
		entityRoot["isDefaultResource"] = m_isDefaultResource;
		m_resourceHashKey.save(entityRoot["resourceHashKey"]);
	}

	virtual void load(const Json::Value & entityRoot) override;

};

//#include "ResourcePtr.inl"