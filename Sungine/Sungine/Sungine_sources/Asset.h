#pragma once

#include "Object.h"

template<typename T>
class AssetHandle
{
	template<typename T>
	friend class AssetLinks;

private:
	T* ptr;
	int index;
	int linkIndex;

public:
	AssetHandle(T* _ptr, int _index)
	{
		ptr = _ptr;
		index = _index;
		linkIndex = ptr->linkManager->addLink(this);
	}
	
	~AssetHandle()
	{
		ptr->linkManager->deleteLink(linkIndex);
	}

	AssetHandle(const AssetHandle& other)
	{
		ptr = other.ptr;
		index = other.index;
		linkIndex = ptr->linkManager->addLink(this);
	}

	AssetHandle& operator=(const AssetHandle& other)
	{
		ptr = other.ptr;
		index = other.index;
		linkIndex = ptr->linkManager->addLink(this);

		return *this;
	}
};

template<typename T>
class AssetPool
{
private:
	std::vector<T> datas;
	int garbageIndex;
	
public:
	AssetHandler<T> alocate()
	{
		if (garbageIndex < datas.size())
		{
			int newIndex = garbageIndex;
			garbageIndex++;
			T* ptr = &datas[newIndex];
			new(ptr) T(); // call constructor

			return AssetHandle<T>(ptr, newIndex);
		}
		else
		{
			return AssetHandle<T>();
		}
	}

	void dealocate(AssetHandler<T>& handler)
	{
		int dataIndex = datasMapping[handler.index];

		if (dataIndex >= 0 && dataIndex < datas.size()
			&& garbageIndex > 0)
		{
			std::iter_swap(datas.begin() + (garbageIndex - 1), datas.begin() + dataIndex);
			garbageIndex--;
			datas[garbageIndex].operator~();

			std::vector<AssetHandle<T>*>& links = datas[dataIndex].linksManager->links;
			for (AssetHandle<T>* link : links)
			{
				link->index = dataIndex;
			}

			(&datas[garbageIndex])->~T();
			generations[index]++;
		}
	}
};

template<typename T>
class AssetLinks
{
public:
	std::vector<AssetHandle<T>*> links;

private:
	int addLink(AssetHandle<T>& handleRef)
	{
		links.push_back(handleRef);
		return links.size();
	}

	void deleteLink(int linkIndex)
	{
		if (links.size() == 1)
		{
			links.pop_back();
		}
		else
		{
			std::iter_swap(links.begin() + linkIndex, links.end() - 1);
			links.pop_back();
			links[linkIndex]->linkIndex = linkIndex;
		}
	}
};

template<typename T>
class Asset : public Object
{
	friend class AssetHanler;

private:
	AssetLinks<T>* linksManager;

public:
	Asset()
	{
		linksManager = new AssetLinks<T>();
	}

	virtual ~Asset()
	{
		delete linksManager;
	}
};