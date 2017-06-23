//#pragma once
//
//
//template<typename T>
//class Handle;
//
//class GenericHandle
//{
//	template<typename T>
//	friend class ObjectPool;
//
//	int classTypeId;
//	unsigned int index;
//	unsigned int generation;
//	IObjectPool* poolRef;
//
//public:
//
//	GenericHandle(IObjectPool* pool)
//		: classTypeId(-1)
//		, index(0)
//		, generation(0)
//		, poolRef(pool)
//	{
//
//	}
//
//	template<typename U>
//	GenericHandle(const Handle<U>& other)
//	{
//		index = other.index;
//		generation = other.generation;
//		classTypeId = other.getStaticClassId();
//		poolRef = other.poolRef;
//	}
//
//	template<typename U>
//	GenericHandle& operator=(const Handle<U>& other)
//	{
//		index = other.index;
//		generation = other.generation;
//		classTypeId = other.getStaticClassId();
//		poolRef = other.poolRef;
//
//		return *this;
//	}
//
//	unsigned int getIndex() const
//	{
//		return index;
//	}
//
//	unsigned int getGeneration() const
//	{
//		return generation;
//	}
//
//	int getClassTypeId() const
//	{
//		return classTypeId;
//	}
//
//	template<typename T>
//	T* resolveRef()
//	{
//		ObjectPool<T>* castedPool = static_cast<ObjectPool<T>*>(poolRef);
//		return castedPool->getRef(*this);
//	}
//};
//
//template<typename T>
//class Handle
//{
//	unsigned int index;
//	unsigned int generation;
//	ObjectPool<T>* poolRef;
//
//public:
//
//	Handle(ObjectPool<T>* pool)
//		: index(0)
//		, generation(0)
//		, poolRef(pool)
//	{
//
//	}
//
//	Handle(const GenericHandle& other)
//	{
//		index = other.getIndex();
//		generation = other.getGeneration();
//		poolRef = static_cast<ObjectPool<T>*>(other.poolRef);
//	}
//
//	Handle<T>& operator=(const GenericHandle& other)
//	{
//		index = other.getIndex();
//		generation = other.getGeneration();
//		poolRef = other.poolRef;
//
//		return *this;
//	}
//
//	unsigned int getIndex() const
//	{
//		return index;
//	}
//
//	unsigned int getGeneration() const
//	{
//		return generation;
//	}
//
//	T* resolveRef()
//	{
//		return poolRef->getRef(*this);
//	}
//};
