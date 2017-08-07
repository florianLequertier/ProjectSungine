#pragma once

#include <new>
#include <assert.h>

class IObjectPool;


template<typename T>
class ObjectPtr
{
	template<typename V>
	friend class ObjectPtr;

private:
	std::weak_ptr<T> m_ptr;
	int m_index;

public:
	ObjectPtr()
		: m_index(-1)
	{}

	ObjectPtr(int index, const std::weak_ptr<T>& ptr)
		: m_ptr(ptr)
		, m_index(index)
	{}

	template<typename U>
	ObjectPtr(const ObjectPtr<U>& other)
		: m_ptr(other.m_ptr)
		, m_index(other.m_index)
	{

	}

	template<typename U>
	ObjectPtr<T>& operator=(const ObjectPtr<U>& other)
	{
		m_ptr = other.m_ptr;
		m_index = other.m_index;

		return *this;
	}

	int getIndex() const
	{
		return m_index;
	}

	std::shared_ptr<T> lock() const
	{
		return m_ptr.lock();
	}

	bool isValid() const
	{
		return m_index > -1 && !m_ptr.expired();
	}

	void reset()
	{
		m_index = -1;
		m_ptr.reset();
	}
};

class IObjectPool
{
public:
	virtual void clear() = 0;
	virtual size_t size() const = 0;
	virtual void resize(int newCapacity) = 0;
	virtual void deallocate(int objectIdx) = 0;
	virtual int getPoolTypeId() const = 0;
};

template<typename T>
class ObjectPool : public IObjectPool
{
private:
	int m_spaceId;
	std::vector<std::shared_ptr<T>> datas;

public:
	ObjectPool(int spaceId, int capacity = 100)
		: m_spaceId(spaceId)
	{
		datas.reserve(100);
	}

	ObjectPtr<T> allocate()
	{
		datas.push_back(std::make_shared<T>());

		const int index = datas.size() - 1;
		return ObjectPtr<T>(index, datas[index]);
	}

	ObjectPtr<T> allocate(const T& other)
	{
		datas.push_back(std::make_shared<T>(other));

		const int index = datas.size() - 1;
		return ObjectPtr<T>(index, datas[index]);
	}

	void deallocate(ObjectPtr<T>& ptr)
	{
		const int objectIndex = ptr.getIndex();
		deallocate(objectIndex);
		ptr.reset();
	}

	template<typename Archive>
	void serealize(Archive& archive)
	{
		archive(datas);
	}

	void clear() override
	{
		datas.clear();
	}

	size_t size() const override
	{
		return datas.size();
	}

	void resize(int newCapacity) override
	{
		datas.reserve(newCapacity);
	}

	void deallocate(int objectIdx) override
	{
		datas[objectIndex].reset();
		std::iter_swap(datas.end() - 1, datas.begin() + objectIndex);
	}

	int getPoolTypeId() const override
	{
		return Object::getStaticClassId<T>();
	}
};

class ObjectSpace
{
protected:
	int m_spaceId;
	std::unordered_map<int, std::shared_ptr<IObjectPool>> m_poolMapping;

public:
	template<typename T>
	ObjectPtr<T> createNew()
	{
		auto pool = getPool<T>();
		if (pool == nullptr)
		{
			std::shared_ptr<ObjectPool<T>> newPool = addNewPool<T>();
			return newPool->allocate();
		}
		else
			return std::static_pointer_cast<T>(pool)->allocate();
	}

	template<typename T>
	ObjectPtr<T> instantiate(const ObjectPtr<T>& otherObject)
	{
		return instantiate(*otherObject.get())
	}

	template<typename T>
	ObjectPtr<T> instantiate(const T& other)
	{
		auto pool = getPool<T>();
		assert(pool != nullptr)
			return pool->instantiate(other)
	}

	template<typename T>
	void destroy(const ObjectPtr<T>& ptr)
	{
		auto pool = getPool<T>();
		assert(pool != nullptr)

		static_cast<ObjectPool<T>>(pool)->deallocate(ptr);
	}

	virtual void clear()
	{
		for (auto& pool : m_poolMapping)
		{
			pool.second->clear();
		}
	}

	// Add a new pool the the object space, and resize it.
	template<typename T>
	std::shared_ptr<ObjectPool<T>> addNewPool(int capacity = 1000)
	{
		assert(m_poolMapping.find(Object::getStaticClassId<T>()) == m_poolMapping.end());

		ObjectPool<T> newPool = std::make_shared<ObjectPool<T>>(m_id, capacity);

		m_poolMapping[Object::getStaticClassId<T>()] = newPool;

		return newPool;
	}

	template<typename T>
	ObjectPool<T>* getPool()
	{
		auto pool = getPool(Object::getStaticClassId<T>());
		if (pool != nullptr)
			return static_cast<ObjectPool<T>*>(getPool(Object::getStaticClassId<T>()));
		else
			return nullptr;
	}

	IObjectPool* getPool(int classId)
	{
		auto found = m_poolMapping.find(classId);
		if (found != m_poolMapping.end())
			return found->second.get();
		else
			return nullptr;
	}

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(m_spaceId, m_poolMapping);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class SaveLoadHelper
{
private:
	ObjectSpace* m_currentObjectSpace;
	bool m_loadingOrSaving;

public:

	SaveLoadHelper()
		: m_loadingOrSaving(false)
	{

	}

	static SaveLoadHelper& instance()
	{
		static SaveLoadHelper* _instance = new SaveLoadHelper();
		return *_instance;
	}

	static ObjectSpace* getCurrentObjectSpace()
	{
		assert(SaveLoadHelper::instance().m_loadingOrSaving);

		return SaveLoadHelper::instance().m_currentObjectSpace;
	}

	static void beginSaveLoad(ObjectSpace* objectSpace)
	{
		SaveLoadHelper::instance().m_currentObjectSpace = objectSpace;
		SaveLoadHelper::instance().m_loadingOrSaving = true;
	}

	static void endSaveLoad()
	{
		SaveLoadHelper::instance().m_currentObjectSpace = nullptr;
		SaveLoadHelper::instance().m_loadingOrSaving = false;;
	}
};
//
//class IObjectPool
//{
//public:
//	virtual void clear() = 0;
//	virtual size_t size() const = 0;
//	virtual void resize(int newCapacity) = 0;
//	virtual int deallocate(int objectIdx, int linkIndex) = 0;
//	virtual int getPoolTypeId() const = 0;
//};
//
//// An object space is a generic class which has the responsability of many objectPools.
//class ObjectSpace
//{
//protected:
//	int m_id;
//	std::map<int, IObjectPool*> m_poolMapping;
//
//public:
//
//	virtual void clear()
//	{
//		for (auto& pool : m_poolMapping)
//		{
//			pool.second->clear();
//		}
//	}
//
//	// Add a new pool the the object space, and resize it.
//	template<typename T>
//	ObjectPool<T>* addNewPool(int capacity = 1000)
//	{
//		assert(m_poolMapping.find(Object::getStaticClassId<T>()) == m_poolMapping.end());
//
//		ObjectPool<T> newPool = new ObjectPool<T>(m_id);
//		newPool.resize(capacity);
//
//		m_poolMapping[Object::getStaticClassId<T>()] = newPool;
//
//		return newPool;
//	}
//
//	template<typename T>
//	static T* getPool()
//	{
//		static ObjectPool<T>* poolRef = m_poolMapping[Object::getStaticClassId<T>()];
//		return poolRef;
//	}
//
//	template<typename T>
//	ObjectPtr<T> createNew()
//	{
//		objectPool<T>* pool = getPool<T>(); // ???
//		return pool->createNewObject();
//	}
//
//	template<typename T>
//	ObjectPtr<T> clone(const ObjectPtr<T>& otherObject)
//	{
//		return otherObject.m_link->cloneObject();
//	}
//
//	template<typename T>
//	ObjectPtr<T> instantiate(const T& other)
//	{
//		return otherObject.m_link->instantiateObject(other);
//	}
//
//	template<typename T>
//	void destroy(ObjectPtr<T> object)
//	{
//		object.m_link->deleteObject();
//	}
//
//};
//
//// A handle object which allow ObjectPtr to access their ObjectPool and perform some operatons.
//// This object link store informations on object (refCount, objectIdx, ...) 
//// which allows it to perform these operations whitout having to store these datas into the real object.
//class ObjectLink
//{
//	template<typename T>
//	friend class ObjectPool;
//
//	template<typename T, typename BaseClass>
//	friend class ObjectPtr;
//
//private:
//
//	IObjectPool* m_poolRef;
//	int m_index;
//	int m_objectIdx;
//	int m_refCount;
//	int m_linkGeneration;
//
//	ObjectLink()
//		: m_index(0)
//		, m_objectIdx(-1)
//		, m_refCount(0)
//		, m_linkGeneration(0)
//	{
//
//	}
//
//	int getObjectTypeId() const
//	{
//		if (m_poolRef != nullptr)
//			return m_poolRef->getPoolTypeId();
//		else
//			return -1;
//	}
//
//	template<typename Archive>
//	void serialize(Archive& archive)
//	{
//		// objectPool and index are automaticallt restored
//		archive(m_objectIdx, m_refCount, m_linkGeneration);
//	}
//
//	void init(IObjectPool* pool, int index)
//	{
//		m_poolRef = pool;
//		m_index = index;
//	}
//
//	void increaseRefCount()
//	{
//		m_refCount++;
//	}
//
//	void decreaseRefCount()
//	{
//		if (m_refCount == 0)
//			return;
//
//		m_refCount--;
//		if (m_refCount == 0)
//		{
//			deleteObject();
//		}
//	}
//
//	void deleteObject()
//	{
//		m_objectIdx = m_poolRef->deallocate(m_objectIdx, m_index);
//		m_linkGeneration++;
//		m_refCount = 0;
//	}
//
//	template<typename T>
//	ObjectPtr<T> cloneObject() const
//	{
//		return static_cast<T>(m_poolRef)->cloneObject(m_objectIdx);
//	}
//
//	template<typename T>
//	ObjectPtr<T> instantiateObject(const T& other)
//	{
//		return static_cast<T>(m_poolRef)->instantiateObject(other);
//	}
//
//	template<typename T>
//	bool isValid(T* ptr)
//	{
//		return refCount > 0 && &static_cast<T>(m_poolRef)[m_objectIdx] == ptr;
//	}
//
//	template<typename T>
//	T* getLinkedObject()
//	{
//		if (m_refCount > 0)
//			return static_cast<T>(m_poolRef)[m_objectIdx];
//		else
//			return nullptr;
//	}
//
//};
//
//template<typename T>
//class ObjectPoolProxy
//{
//private:
//	IObjectPool* m_target;
//
//public:
//	ObjectPoolProxy(IObjectPool* target)
//		: m_target(target)
//	{}
//
//	size_t size() const override
//	{
//		return m_target->m_data.size();
//	}
//
//	const T& operator[](int index) const
//	{
//		return *static_cast<T*>(&m_target->m_datas[index]);
//	}
//
//	T& operator[](int index)
//	{
//		return *static_cast<T*>(&m_target->m_datas[index]);
//	}
//};
//
//// A pool which can allocate and deallocate objects. It will store objects contiguously avoiding cache miss (m_datas), 
//// and will store handles to access and perform operations on these objects (m_links) (these ones are not stored contiguously).
//template<typename T>
//class ObjectPool : public IObjectPool
//{
//public:
//	static bool g_isRegister;
//
//private:
//	int m_spaceId = -1;
//	std::vector<T> m_datas;
//	std::vector<ObjectLink> m_links;
//	int m_nextAvailableDataIdx = 0;
//	int m_firstAvailableLinkIdx = 0;
//
//public:
//	ObjectPool(int spaceId, int initialCapacity = 1000)
//	{
//		resize(initialCapacity);
//	}
//
//	void clear() override
//	{
//		// Call destructors
//		for (T& data : m_datas)
//		{
//			(&data)->~T();
//		}
//
//		// reset linking
//		for (int i = 0; i < m_links.size(); i++)
//		{
//			m_links[0].init(this, i);
//		}
//
//		// Reset index
//		m_nextAvailableDataIdx = 0;
//		m_firstAvailableLinkIdx = 0;
//	}
//
//	void resize(int newCapacity) override
//	{
//		int lastCapacity = m_datas.size();
//
//		while (m_firstAvailableLinkIdx >= newCapacity && m_firstAvailableLinkIdx != -1)
//		{
//			m_firstAvailableLinkIdx = m_links[m_firstAvailableLinkIdx].m_objectIdx;
//		}
//
//		m_datas.resize(newCapacity);
//		m_links.resize(newCapacity);
//
//		if (m_nextAvailableDataIdx > newCapacity)
//		{
//			m_nextAvailableDataIdx = newCapacity;
//		}
//
//		for (int i = lastCapacity; i < newCapacity; i++)
//		{
//			m_links[i].init(this, i);
//		}
//	}
//
//	int getPoolTypeId() const override
//	{
//		return Object::getStaticClassId<T>();
//	}
//
//	ObjectLink<T>* getLinkPtr(int linkIndex)
//	{
//		return m_links[linkIndex];
//	}
//
//	template<typename Archive>
//	void serialize(Archive& archive)
//	{
//		archive(m_datas, m_links, m_nextAvailableDataIdx, m_firstAvailableLinkIdx);
//	}
//
//	size_t size() const override
//	{
//		return m_data.size();
//	}
//
//	const T& operator[](int index) const
//	{
//		return m_datas[index];
//	}
//
//	T& operator[](int index)
//	{
//		return m_datas[index];
//	}
//
//	typename std::vector<T>::iterator begin()
//	{
//		return m_datas.begin();
//	}
//
//	typename std::vector<T>::const_iterator begin() const
//	{
//		return m_datas.begin();
//	}
//
//	typename std::vector<T>::iterator end()
//	{
//		return m_datas.end();
//	}
//
//	typename std::vector<T>::const_iterator end() const
//	{
//		return m_datas.end();
//	}
//
//	// Private functions accessible via ObjectLink
//private:
//
//	// Release the link and delete the object. Return the index pointing to the next free link.
//	int deallocate(int objectIdx, int linkIndex) override
//	{
//		// Dealocate pool object
//		assert(objectIdx >= 0
//			&& objectIdx < m_nextAvailableDataIdx
//			&& m_nextAvailableDataIdx < m_datas.size());
//
//		(&m_datas[objectIdx])->~T();
//
//		std::iter_swap(m_datas.begin() + m_nextAvailableDataIdx, m_datas.begin() + m_nextAvailableDataIdx);
//		m_nextAvailableDataIdx--;
//
//		// Release link
//		int lastIdx = m_firstAvailableLinkIdx;
//		int idx = linkIndex;//std::distance(&m_links[0], link);
//		m_firstAvailableLinkIdx = idx;
//		return lastIdx;
//	}
//
//	// Use the next free link in the links array.
//	ObjectLink* useNextLink(int objectIdx)
//	{
//		ObjectLink<T>* linkRef = &m_links[m_firstAvailableLinkIdx];
//		m_firstAvailableLinkIdx = linkRef->m_objectIdx;
//		linkRef->m_objectIdx = objectIdx;
//		linkRef->m_linkGeneration++;
//		return linkRef;
//	}
//
//	ObjectPtr<T> allocate()
//	{
//		assert(m_nextAvailableDataIdx < m_datas.size());
//		T* ref = &m_datas[m_nextAvailableDataIdx];
//		ObjectLink<T>* linkRef = useNextLink(m_nextAvailableDataIdx);
//		m_nextAvailableDataIdx++;
//
//		new(ref) T(); // call constructor with placement new
//
//		return ObjectPtr<T>(ref, linkRef);
//	}
//
//	ObjectPtr<T> allocate(const T& other)
//	{
//		assert(m_nextAvailableDataIdx < m_datas.size());
//		T* ref = &m_datas[m_nextAvailableDataIdx];
//		ObjectLink<T>* linkRef = useNextLink(m_nextAvailableDataIdx);
//		m_nextAvailableDataIdx++;
//
//		new(ref) T(other); // call copy constructor with placement new
//
//		return ObjectPtr<T>(ref, linkRef);
//	}
//
//	// Allow you to create a new object in the pool
//	ObjectPtr<T> createNewObject()
//	{
//		return allocate();
//	}
//
//	// Allow you to create an object in the pool, copied from a other object in the pool
//	ObjectPtr<T> cloneObject(int otherObjectIdx)
//	{
//		assert(otherObjectIdx >= 0
//			&& otherObjectIdx < m_nextAvailableDataIdx)
//			const T& other = m_datas[otherObjectIdx];
//
//		return instantiateObject(other);
//	}
//
//	// Allow you to create an object in the pool, copied from a other object given in parameter
//	ObjectPtr<T> instantiateObject(const T& other)
//	{
//		return allocate(other);
//	}
//};
//
//// A safe and serializable pointer to an object.
//template<typename T, typename BaseClass = T>
//class ObjectPtr
//{
//private:
//	T* m_data;
//	ObjectLink* m_link;
//
//public:
//	ObjectPtr()
//		: m_data(nullptr)
//		, m_link(nullptr)
//	{
//	}
//
//	ObjectPtr(T* data, ObjectLink<T>* link)
//		: m_data(data)
//		, m_link(link)
//	{
//		m_link->increaseRefCount();
//	}
//
//	ObjectPtr(T* data, ObjectSpace& space)
//	{
//		auto pool = space.getPool<BaseClass>();
//		m_link = pool->getLinkFromObject(data);
//	}
//
//	template<typename U>
//	ObjectPtr(const ObjectPtr<U>& other)
//	{
//		m_data = static_cast<T>(other.m_data);
//		m_link = other.m_link;
//		m_link->increaseRefCount();
//	}
//
//	template<typename U>
//	ObjectPtr<T>& operator=(const ObjectPtr<U>& other)
//	{
//		if (m_data == other.m_data)
//		{
//			m_link = other.m_link;
//			return *this;
//		}
//
//		m_data = static_cast<T>(other.m_data);
//		m_link = other.m_link;
//		m_link->increaseRefCount();
//
//		return *this;
//	}
//
//	bool operator==(const T* ptr) const
//	{
//		return ptr == m_data;
//	}
//
//	bool operator==(const ObjectPtr<T>& other) const
//	{
//		return (other.m_data == m_data && other.m_link = m_link);
//	}
//
//	template<typename U>
//	bool operator==(const ObjectPtr<U>& other) const
//	{ 
//		return (other.m_data == m_data && other.m_link = m_link);
//	}
//
//	T* operator->() const
//	{
//		return m_data;
//	}
//
//	virtual ~ObjectPtr()
//	{
//		reset();
//	}
//
//	void reset()
//	{
//		if(m_link != nullptr)
//			m_link->decreaseRefCount();
//
//		m_link = nullptr;
//		m_data = nullptr;
//	}
//
//	template<typename Archive>
//	void save(Archive& archive)
//	{
//		if (m_data != nullptr && m_link != nullptr)
//			save(m_data->getClassName(), m_link->m_index);
//		else
//			save("", -1);
//	}
//
//	template<typename Archive>
//	void load(Archive& archive)
//	{
//		ObjectSpace* space = SaveLoadHelper::getCurrentObjectSpace();
//
//		if (space != nullptr)
//		{
//			std::string className = "";
//			int linkIdx = -1;
//			load(className, linkIdx);
//
//			// Are we loading a "null" pointer ?
//			if (linkIdx != -1)
//			{
//				assert(className != ""); // mayday we have a problem !!!
//
//				IObjectPool* pool = space.getPool(Object::getClassIdFromName(className));
//				// or : 
//				// IObjectPool* pool = space.getPool<BaseClass>();
//				// Should work too
//				if (pool != nullptr)
//				{
//					m_link = pool.getLink(linkIdx);
//					if(m_link != nullptr)
//						m_data = m_link->getLinkedObject();
//				}
//			}
//			else
//			{
//				m_data = nullptr;
//				m_link = nullptr;
//			}
//		}
//	}
//
//	bool isValid() const
//	{
//		return m_link != nullptr && m_link->isValid();
//	}
//
//	T* getPtr()
//	{
//		if (m_data != nullptr && !isValid())
//			m_data = nullptr;
//		return m_data;
//	}
//
//	ObjectPtr<T> clonePointedObject() const
//	{
//		return m_link->cloneObject<BaseClass>();
//	}
//};