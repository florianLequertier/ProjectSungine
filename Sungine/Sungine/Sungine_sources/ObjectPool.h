#pragma once

#include <new>
#include <assert.h>

#include "Handle.h"

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


// A handle object which allow ObjectPtr to access their ObjectPool and perform some operatons.
// This object link store informations on object (refCount, objectIdx, ...) 
// which allows it to perform these operations whitout having to store these datas into the real object.
template<typename T>
class ObjectLink
{
	template<typename T>
	friend class ObjectPool;

	template<typename T>
	friend class ObjectPtr;

private:

	ObjectPool<T>* m_poolRef;
	int m_index;
	int m_objectIdx;
	int m_refCount;
	int m_linkGeneration;

	template<typename Archive>
	void serialize(Archive& archive)
	{
		// objectPool and index are automaticallt restored
		archive(m_objectIdx, m_refCount, m_linkGeneration);
	}

	void init(const ObjectPool<T>* pool, int index)
	{
		m_poolRef = pool;
		m_index = index;
	}

	void increaseRefCount()
	{
		m_refCount++;
	}

	void decreaseRefCount()
	{
		if (m_refCount == 0)
			return;

		m_refCount--;
		if (m_refCount == 0)
		{
			deleteObject();
		}
	}

	void deleteObject()
	{
		m_objectIdx = m_poolRef->deleteObject(m_objectIdx, m_index);
		link->m_objectGeneration++;
		m_refCount = 0;

		assert(m_refCount == 0);
	}

	ObjectPtr<T> cloneObject()
	{
		return m_poolRef->cloneObject(m_objectIdx);
	}

	ObjectPtr<T> instantiateObject(const T& other)
	{
		return m_poolRef->instantiateObject(other);
	}

	bool isValid(T* ptr)
	{
		return refCount > 0 && &m_poolRef[m_objectIdx] == ptr;
	}

	T* getLinkedObject()
	{
		if (m_refCount > 0)
			return m_poolRef[m_objectIdx];
		else
			return nullptr;
	}

};

// An object space is a generic class which has the responsability of many objectPools.
class ObjectSpace
{
	int m_id;
	std::map<int, IObjectPool*> m_poolMapping;

public:

	// Add a new pool the the object space, and resize it.
	template<typename T>
	void addNewPool(int capacity = 1000)
	{
		assert(m_poolMapping.find(Object::getStaticClassId<T>()) == m_poolMapping.end());

		ObjectPool<T> newPool = new ObjectPool<T>(m_id);
		newPool.resize(capacity);

		m_poolMapping[Object::getStaticClassId<T>()] = newPool;
	}

	template<typename T>
	static T* getPool()
	{
		static ObjectPool<T>* poolRef = m_poolMapping[Object::getStaticClassId<T>()];
		return poolRef;
	}

};

class IObjectPool
{
	size_t size() const;
	void resize(int newCapacity);
	void deallocate(int objectIdx, int linkIndex);
};

// A pool which can allocate and deallocate objects. It will store objects contiguously avoiding cache miss (m_datas), 
// and will store handles to access and perform operations on these objects (m_links) (these ones are not stored contiguously).
template<typename T>
class ObjectPool
{
public:
	static bool g_isRegister;

private:
	int m_spaceId = -1;
	std::vector<T> m_datas;
	std::vector<ObjectLink<T>> m_links;
	int m_nextAvailableDataIdx = 0;
	int m_firstAvailableLinkIdx = 0;

public:
	ObjectPool(int spaceId, int initialCapacity = 1000)
	{
		resize(initialCapacity);
	}

	size_t size() const
	{
		return m_data.size();
	}

	void resize(int newCapacity)
	{
		int lastCapacity = m_datas.size();

		while (m_firstAvailableLinkIdx >= newCapacity && m_firstAvailableLinkIdx != -1)
		{
			m_firstAvailableLinkIdx = m_links[m_firstAvailableLinkIdx].m_objectIdx;
		}

		m_datas.resize(newCapacity);
		m_links.resize(newCapacity);

		if (m_nextAvailableDataIdx > newCapacity)
		{
			m_nextAvailableDataIdx = newCapacity;
		}

		for (int i = lastCapacity; i < newCapacity; i++)
		{
			m_links[i].init(this, i);
		}
	}

	const T& operator[](int index) const
	{
		return m_datas[index];
	}

	const T& operator[](int index)
	{
		return m_datas[index];
	}

	ObjectLink<T>* getLinkPtr(int linkIndex)
	{
		return m_links[linkIndex];
	}

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(m_datas, m_links, m_nextAvailableDataIdx, m_firstAvailableLinkIdx);
	}

	// Private functions accessible via ObjectLink
private:

	// Release the link and delete the object. Return the index pointing to the next free link.
	void deallocate(int objectIdx, int linkIndex)
	{
		// Dealocate pool object
		assert(objectIdx >= 0
			&& objectIdx < m_nextAvailableDataIdx
			&& m_nextAvailableDataIdx < m_datas.size());

		(&m_datas[objectIdx])->~T();

		std::iter_swap(m_datas.begin() + m_nextAvailableDataIdx, m_datas.begin() + m_nextAvailableDataIdx);
		m_nextAvailableDataIdx--;

		// Release link
		int lastIdx = m_firstAvailableLinkIdx;
		int idx = linkIndex;//std::distance(&m_links[0], link);
		m_firstAvailableLinkIdx = idx;
		return lastIdx;
	}

	// Use the next free link in the links array.
	ObjectLink<T>* useNextLink(int objectIdx)
	{
		ObjectLink<T>* linkRef = &m_links[m_firstAvailableLinkIdx];
		m_firstAvailableLinkIdx = linkRef->m_objectIdx;
		linkRef->m_objectIdx = objectIdx;
		linkRef->m_linkGeneration++;
		return linkRef;
	}

	ObjectPtr<T> allocate()
	{
		assert(m_nextAvailableDataIdx < m_datas.size());
		T* ref = &m_datas[m_nextAvailableDataIdx];
		ObjectLink<T>* linkRef = useNextLink(m_nextAvailableDataIdx);
		m_nextAvailableDataIdx++;

		new(ref) T(); // call constructor with placement new

		return ObjectPtr<T>(ref, linkRef);
	}

	ObjectPtr<T> allocate(const T& other)
	{
		assert(m_nextAvailableDataIdx < m_datas.size());
		T* ref = &m_datas[m_nextAvailableDataIdx];
		ObjectLink<T>* linkRef = useNextLink(m_nextAvailableDataIdx);
		m_nextAvailableDataIdx++;

		new(ref) T(other); // call copy constructor with placement new

		return ObjectPtr<T>(ref, linkRef);
	}

	// Allow you to create a new object in the pool
	ObjectPtr<T> createNewObject()
	{
		return allocate();
	}

	// Allow you to create an object in the pool, copied from a other object in the pool
	ObjectPtr<T> cloneObject(int otherObjectIdx)
	{
		assert(otherObjectIdx >= 0
			&& otherObjectIdx < m_nextAvailableDataIdx)
			const T& other = m_datas[otherObjectIdx];

		return instantiateObject(other);
	}

	// Allow you to create an object in the pool, copied from a other object given in parameter
	ObjectPtr<T> instantiateObject(const T& other)
	{
		return allocate(other);
	}
};

// A safe and serializable pointer to an object.
template<typename T>
class ObjectPtr
{
private:
	T* m_data;
	ObjectLink<T>* m_link;

public:
	ObjectPtr(T* data, ObjectLink<T>* link)
		: m_data(data), m_link(link)
	{
		m_link->increaseRefCount();
	}

	~ObjectPtr()
	{
		m_link->decreaseRefCount();
		m_link = nullptr;
		m_data = nullptr;
	}

	template<typename Archive>
	void save(Archive& archive)
	{
		load(m_link->m_index);
	}

	template<typename Archive>
	void load(Archive& archive)
	{
		ObjectSpace* space = SaveLoadHelper::getCurrentObjectSpace();

		if (space != nullptr)
		{
			ObjectPool<T>* pool = space.getPool<T>();
			if (pool != nullptr)
			{
				int linkIdx;
				load(linkIdx);

				m_link = pool.getLink(linkIdx);
				if(m_link != nullptr)
					m_data = m_link->getLinkedObject();
			}
		}
	}

	bool isValid()
	{
		return m_link != nullptr && m_link->isValid();
	}

	T* getPtr()
	{
		if (m_data != nullptr && !isValid())
			m_data = nullptr;
		return m_data;
	}

	static ObjectPtr<T> createNew()
	{
		objectPool<T>* pool = getPool<T>(); // ???
		return pool->createNewObject();
	}

	static ObjectPtr<T> clone(const ObjectPtr<T>& otherObject)
	{
		return otherObject.m_link->cloneObject();
	}

	static ObjectPtr<T> instantiate(const T& other)
	{
		return otherObject.m_link->instantiateObject(other);
	}

	static void delete(ObjectPtr<T> object)
	{
		object.m_link->deleteObject();
	}
};



//class IObjectPool
//{
//public:
//	virtual void* allocate(GenericHandle& outHandle) = 0;
//	virtual void deallocate(int index, int generation) = 0;
//	virtual void deallocateAll() = 0;
//	virtual void update() = 0;
//	virtual void resize(size_t newSize) = 0;
//};
//
//template<typename T>
//class ObjectPool : public IObjectPool
//{
//	std::vector<T> datas;
//	std::vector<unsigned int> generations;
//	int garbageIndex;
//
//public:
//
//	ObjectPool()
//		: garbageIndex(0)
//	{
//		resize(1000);
//	}
//
//	void resize(size_t newSize) override
//	{
//		datas.resize(newSize);
//		generations.resize(newSize, 0);
//	}
//
//	T* getRef(const Handle<T>& handle)
//	{
//		assert(datas.size() == generations.size());
//
//		if (handle.getIndex() >= 0 && handle.getIndex() < datas.size()
//			&& handle.getGeneration() == generations[handle.getIndex()])
//		{
//			return &datas[handle.getIndex()];
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//	void update() override;
//
//	void* allocate(GenericHandle& outHandle) override
//	{
//		if (garbageIndex < datas.size())
//		{
//			int newIndex = garbageIndex;
//			garbageIndex++;
//			T* ptr = &datas[newIndex];
//			new(ptr) T(); // call constructor
//
//			outHandle.classTypeId = Object::getStaticClassId<T>();
//			outHandle.generation = generations[newIndex];
//			outHandle.index = newIndex;
//			return ptr;
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//	template<typename... Args>
//	void* allocate(GenericHandle& outHandle, Args&&... args) override
//	{
//		if (garbageIndex < datas.size())
//		{
//			int newIndex = garbageIndex;
//			garbageIndex++;
//			T* ptr = &datas[newIndex];
//			new(ptr) T(std::forward<Args>(args)...); // call constructor
//
//			outHandle.classTypeId = Object::getStaticClassId<T>();
//			outHandle.generation = generations[newIndex];
//			outHandle.index = newIndex;
//			return ptr;
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//	void deallocate(int index, int generation) override
//	{
//		assert(generation == generations[index]);
//
//		if (index >= 0 && index < datas.size()
//			&& garbageIndex > 0)
//		{
//			std::iter_swap(datas.begin() + (garbageIndex - 1), datas.begin() + index);
//			garbageIndex--;
//
//			(&datas[garbageIndex])->~T();
//			generations[index]++;
//		}
//	}
//
//	void deallocateAll() override
//	{
//		for (int i = 0; i < garbageIndex; i++)
//		{
//			(&datas[i])->~T();
//		}
//		for (int i = 0; i < garbageIndex; i++)
//		{
//			generations[i]++;
//		}
//		garbageIndex = 0;
//	}
//
//	void* copy(const Handle<T>& modelHandle, GenericHandle& outHandle) override
//	{
//		if (garbageIndex < datas.size()
//			&& generations[modelHandle.index] == modelHandle.generation)
//		{
//			int newIndex = garbageIndex;
//			garbageIndex++;
//			T* ptr = &datas[newIndex];
//			const T& other = datas[modelHandle.index];
//			new(ptr) T(other); // call copy constructor
//
//			outHandle.classTypeId = Object::getStaticClassId<T>();
//			outHandle.generation = generations[newIndex];
//			outHandle.index = newIndex;
//			return ptr;
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//
//	void* copy(const T& model, GenericHandle& outHandle) override
//	{
//		if (garbageIndex < datas.size()
//			&& generations[modelHandle.index] == modelHandle.generation)
//		{
//			int newIndex = garbageIndex;
//			garbageIndex++;
//			T* ptr = &datas[newIndex];
//			new(ptr) T(model); // call copy constructor
//
//			outHandle.classTypeId = Object::getStaticClassId<T>();
//			outHandle.generation = generations[newIndex];
//			outHandle.index = newIndex;
//			return ptr;
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//};
//
//template<typename T>
//inline void ObjectPool<T>::update()
//{
//}