#pragma once

#include "FileHandler.h"
#include "Asset.h"

template<typename T>
class AssetHandle
{
	template<typename T>
	friend class AssetLinks;

private:
	T* ptr;
	AssetLink<T>* link;

public:
	AssetHandle(T* _ptr, AssetLink<T>* _link)
	{
		ptr = _ptr;
		link = _link;
		link->addLink(this);
	}

	~AssetHandle()
	{
		if(link != nullptr)
			link->deleteLink(linkIndex);
	}

	AssetHandle(const AssetHandle& other)
	{
		ptr = other.ptr;
		link = other.link;
		link->addLink(this);
	}

	AssetHandle& operator=(const AssetHandle& other)
	{
		ptr = other.ptr;
		link = other.link;
		link->addLink(this);

		return *this;
	}

	template<typename Archive>
	void save(Archive& archive)
	{
		int _index = -1;
		int _dataIdx = -1;
		AssetId _assetId;

		if (link != nullptr)
		{
			_index = link->index;
			_index = link->dataIdx;
			_assetId = link->id;
		}

		save(_index, _dataIdx, _assetId);
	}

	template<typename Archive>
	void load(Archive& archive)
	{
		int _index = -1;
		int _dataIdx = -1;
		AssetId _assetId;

		load(_index, _dataIdx, _assetId);

		if (_index != -1 && _dataIndex != -1)
		{
			link = AssetManager::instance().getPool<T>()->getLink(_assetId);
			ptr = link->getLinkedObject();
		}
	}

	T* getPtr()
	{
		return ptr;
	}
};

class IAssetPool
{
public:
	virtual void loadAsset(const FileHandler::CompletePath& assetPath) = 0;
};

template<typename T>
class AssetPool : public IAssetPool
{
private:
	// It may be interesting to save a GUID in a meta file instead of a string representing the path. With this methode, links won't break if the meta file is along the data file.
	std::unordered_map<AssetId, int> dataLinkMapping; // id -> index to link

	std::unordered_map<std::string, AssetHandle<T>> defaults;

	std::vector<T> datas;
	std::vector<int> datasToLinkIdx;
	std::vector<AssetLinks<T>> links;
	int nextFreeDataIdx;
	int nextFreeLinkIdx;

	// The last id (i.e biggest) of an asset inside this pool
	AssetId lastAssetId;

public:
	AssetPool()
		: nextFreeDataIdx(0)
		, nextFreeLinkIdx(0)
	{
		for (int i = 0; i < links.size(); i++)
		{
			links[i].init(i);
		}

		resize(1000);
	}

	// Make sure that the given id will be restored properly.
	AssetId restoreUniqueId(AssetId assetId)
	{
		// make sure we are working with an id which handle an asset which can be in this pool.
		assert(Object::getStaticClassId<T>() == assetId.type);

		// If an asset already own this id, it will modify the id of this asset.
		if (assetId.id < Asset::lastGeneratedId())
		{
			// We generate a new Id for the asset which was already using this id.
			datas[links[dataLinkMapping[assetId.id]].dataIdx].id = Asset::generateUniqueId();
			return id;
		}
		else
			return id;
	}

	void resize(int newCapacity)
	{
		int oldSize = data.size();
		datas.resize(newCapacity);
		datasToLinkIdx.resize(newCapacity);
		links.resize(newCapacity);

		for (int i = oldSize; i < newCapacity; i++)
		{
			links[i].init(i);
		}
	}

	AssetHandle<T> allocate()
	{
		if (nextFreeDataIdx < datas.size())
		{
			// create object
			const int newIndex = nextFreeDataIdx;
			nextFreeDataIdx++;
			T* ptr = &datas[newIndex];
			new(ptr) T(); // call constructor

			// setup links
			const int linkIndex = nextFreeLinkIdx;
			AssetLinks<T>* link = links[linkIndex];
			nextFreeLinkIdx = link->dataIdx;
			link->pointToData(linkIndex, newIndex, ptr->getAssetId());

			// setup data to link mapping
			datasToLinkIdx[newIndex] = nextFreeLinkIdx;

			return AssetHandle<T>(ptr, link);
		}
		else
		{
			return AssetHandle<T>();
		}
	}

	// This allocation function is only used when we are loading the assets.
	// When an asset was already created in a previous session, we restore its previous datas and id.
	AssetHandle<T> allocate(int linkIndex, AssetId id)
	{
		if (nextFreeDataIdx < datas.size())
		{
			// create object
			const int newIndex = nextFreeDataIdx;
			nextFreeDataIdx++;
			T* ptr = &datas[newIndex];
			new(ptr) T(id); // call constructor

			if (links[linkIndex].dataIdx > -1)
			{
				AssetLinks<T>* link = &links[nextFreeLinkIdx];
				nextFreeLinkIdx = link->dataIdx;
				AssetLinks<T>::swapLinks(links[linkIndex], link);
			}

			// setup links
			AssetLinks<T>* link = &links[linkIndex];
			nextFreeLinkIdx = link->dataIdx;
			link->pointToData(linkIndex, newIndex, id);

			// setup data to link mapping
			datasToLinkIdx[newIndex] = nextFreeLinkIdx;

			return AssetHandle<T>(ptr, link);
		}
		else
		{
			return AssetHandle<T>();
		}
	}

	AssetHandle<T> allocate(const T& objectData)
	{
		if (nextFreeDataIdx < datas.size())
		{
			// create object
			const int newIndex = nextFreeDataIdx;
			nextFreeDataIdx++;
			T* ptr = &datas[newIndex];
			new(ptr) T(assetData); // call constructor

			// setup links
			const int linkIndex = nextFreeLinkIdx;
			AssetLinks<T>* link = links[linkIndex];
			nextFreeLinkIdx = link->dataIdx;
			link->pointToData(linkIndex, newIndex, ptr->getAssetId());

			// setup data to link mapping
			datasToLinkIdx[newIndex] = nextFreeLinkIdx;

			return AssetHandle<T>(ptr, link);
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
			// reset links
			handler.link->reset();
			handler.link->dataIdx = nextFreeLinkIdx;
			nextFreeLinkIdx = std::distance(links, &handler.link); // ???

			// destroy object
			std::iter_swap(datas.begin() + (nextFreeDataIdx - 1), datas.begin() + dataIndex);
			nextFreeDataIdx--;
			(&datas[nextFreeDataIdx])->~T();
		}
	}

	// Take an object (ex : a Mesh, created with "new Mesh()") and insert it to the asset system.
	// This will call the saveAssetToFile function of the asset.
	AssetHandle<T> createAsset(const T& object, const FileHandler::CompletePath& assetPath)
	{
		AssetHandle<T> newHandle = allocate(object);
		newHandle->createNewAssetFile(assetPath);
	}

	// When you cook the assets, you will serialize all the assets in one file
	template<typename Archive>
	void cook(Archive& archive)
	{
		archive(nextFreeDataIdx, nextFreeLinkIdx, datas, links);
	}

	// Save the mapping, and save the assets which can be saved in a file.
	template<typename Archive>
	void save(Archive& archive)
	{
		archive(dataMapping);
		for (auto& data : datas)
		{
			data.saveToFile();
		}
	}

	// Load only the mapping
	template<typename Archive>
	void load(Archive& archive)
	{
		archive(dataMapping);
	}

	// Create a new asset in memory and load its datas from a file.
	void loadAsset(const FileHandler::CompletePath& assetPath) override
	{
		AssetHandle<T> newAssetHandle;

		FileHandler::CompletePath assetMetaPath = assetPath;
		assetMetaPath.replaceExtension("meta");
		
		AssetId fileId = -1;
		// Read metas, try to get the file id
		if(FileHandler::fileExists(assetMetaPath))
		{
			Json::Value readMetaFile;
			AssetManager::loadMetaFile(metaPath, readMetaFile);
			fileId = (AssetID)readMetaFile["id"].asInt();
		}

		// We check if we have a valid mapping for this id.
		auto& findIt = dataLinkMapping.find(fileId);
		if (findIt != dataLinkMapping.end())
		{
			// If we have a valid mapping, we will allocate the new asset at its old location
			newAssetHandle = allocate(findIt->first, restoreUniqueId(fileId));
		}
		else
		{
			// If we don't have a mapping yet, we allocate in a new location
			newAssetHandle = allocate();
		}

		T* newAsset = newAssetHandle.getPtr();
		newAsset->loadFromFile(assetPath);
	}

	AssetLink<T>* getLink(const AssetId& assetId)
	{
		auto found = dataLinkMapping.find(assetId);
		if (found != dataLinkMapping.end())
		{
			return &links[found.second()];
		}
	}

	bool getDefaultAsset(const std::string& assetName, AssetHandle<T>& outHandle)
	{
		auto found = defaults.find(assetName);
		if (found != defaults.end())
		{
			outHandle = found.second;
			return true;
		}
		else
		{
			false;
		}
	}

	bool getAsset(const AssetId& assetId, AssetHandle<T>& outHandle)
	{
		auto found = dataLinkMapping.find(assetId);
		if (found != dataLinkMapping.end())
		{
			outHandle = found.second;
			return true;
		}
		else
		{
			return false;
		}
	}
};

template<typename T>
class AssetLinks
{
public:
	int index;
	int dataIdx;
	AssetId id;
	std::vector<AssetHandle<T>*> links;

	static void swapLinks(AssetLink<T>* a, AssetLink<T>* b)
	{
		int c_dtaIdx = a.dataIdx;
		a.dataIdx = b.dataIdx;
		b.dataIdx = c.dataIdx;

		std::vector<AssetHandle<T>*> c_links;
		c_links = a.links;
		a.links = b.links;
		b.links = c_links;
	}

private:

	void init(int _index)
	{
		index = _index;
		dataIdx = _index + 1;
	}

	void pointToData(int selfIndex, int dataIndex, AssetId& dataId)
	{
		index = selfIndex;
		dataIdx = dataIndex;
		id = dataId;
		links.clear();
	}

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

	void reset()
	{
		for (auto link : links)
		{
			link->reset();
		}

		links.clear();
	}

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(index, dataIdx, links);
	}
};

