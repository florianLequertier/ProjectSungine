#pragma once

#include <jsoncpp/json/json.h>

#include "FileHandler.h"
#include "Asset.h"

class BaseAssetHandle
{
	friend class AssetLinks;

protected:
	int linkIndex;
public:
	virtual void reset() = 0;
};

template<typename T>
class AssetHandle : public BaseAssetHandle
{
	friend class AssetLinks;

private:
	T* m_ptr;
	AssetLinks* m_links;
	int m_linkIndex;

public:
	AssetHandle()
	{
		m_ptr = nullptr;
		m_links = nullptr;
		m_linkIndex = -1;
	}

	AssetHandle(T* _ptr, AssetLinks* _links)
	{
		m_ptr = _ptr;
		m_links = _links;
		m_linkIndex = links->addLink(this);
	}

	~AssetHandle()
	{
		reset();
	}

	AssetHandle(const AssetHandle& other)
	{
		m_ptr = other.m_ptr;
		m_links = other.m_links;
		m_linkIndex = m_links->addLink(this);
	}

	// auto Cast
	template<typename U>
	AssetHandle(const AssetHandle<U>& other)
	{
		m_ptr = static_cast<T*>(other.m_ptr);
		m_links = static_cast<AssetLinks*>(other.m_links);
		m_linkIndex = m_links->addLink(this);
	}

	AssetHandle& operator=(const AssetHandle& other)
	{
		m_ptr = other.m_ptr;
		m_links = other.m_links;
		m_linkIndex = m_links->addLink(this);

		return *this;
	}

	void reset() override
	{
		if (m_ptr != nullptr)
			m_ptr = nullptr;
		if (m_links != nullptr)
			m_links->deleteLink(m_linkIndex);

		m_linkIndex = -1;
	}

	template<typename Archive>
	void save(Archive& archive) const
	{
		AssetId _assetId;

		if (m_links != nullptr)
		{
			_assetId = m_links->id;
		}

		save(_assetId);
	}

	template<typename Archive>
	void load(const Archive& archive)
	{
		AssetId _assetId;

		load(_assetId);

		initFromAssetId(_assetId);
	}

	// Specialisation for JsonCPP
	void save(Json::Value& archive) const
	{
		AssetId _assetId;

		if (link != nullptr)
		{
			_assetId = m_links->id;
		}

		archive["assetId"] = _assetId;
	}

	void load(const Json::Value& archive)
	{
		AssetId _assetId;

		_assetId = archive["assetId"];

		initFromAssetId(_assetId);
	}

	T* getPtr()
	{
		return m_ptr;
	}

	bool isValid() const
	{
		return m_ptr != nullptr;
	}

	T* operator->() const
	{
		return m_ptr;
	}

private:
	void initFromAssetId(const AssetId& assetId)
	{
		if (assetId.isValid())
		{
			AssetPool<T>* pool = AssetManager::instance().getPool<T>();
			assert(pool != nullptr);

			m_links = pool->getLink(assetId);

			if (m_linkIndex != -1)
				m_links->removeLink(m_linkIndex);
			m_linkIndex = m_links->addLink(this);

			m_ptr = m_links->datas[m_links->dataIdx];
		}
	}
};

class IAssetPool
{
public:
	virtual Asset* loadAsset(const FileHandler::CompletePath& assetPath) = 0;
	virtual bool assetExists(const AssetId& assetId) const = 0;
	virtual Asset* getAssetPtr(const AssetId& assetId) const = 0;
	virtual bool dealocate(const AssetId& assetId) = 0;
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
	std::vector<AssetLinks> links;
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

	std::vector<T>::const_iterator getDataIteratorBegin() const
	{
		return datas.begin();
	}

	std::vector<T>::const_iterator getDataIteratorEnd() const
	{
		return datas.begin() + nextFreeDataIdx;
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

	bool dealocate(AssetLinks& link)
	{
		const int dataIndex = datasMapping[link.dataIdx];

		if (dataIndex >= 0 && dataIndex < datas.size()
			&& garbageIndex > 0)
		{
			// reset links
			link->reset();
			link->dataIdx = nextFreeLinkIdx;
			nextFreeLinkIdx = link->index;// std::distance(links, &link); // ???

			// destroy object
			std::iter_swap(datas.begin() + (nextFreeDataIdx - 1), datas.begin() + dataIndex);
			nextFreeDataIdx--;
			(&datas[nextFreeDataIdx])->~T();

			return true;
		}
		return false;
	}

	bool dealocate(const AssetHandle<T>& handler)
	{
		return dealocate(handler.link);
	}

	bool dealocate(const AssetId& assetId) override 
	{
		auto foundIdx = dataLinkMapping.find(assetId);
		if (foundIdx != dataLinkMapping.end())
		{
			const AssetLinks& link = links[foundIdx->second];
			dealocate(link);
		}
		return false;
	}

	// Take an object (ex : a Mesh, created with "new Mesh()") and insert it to the asset system.
	// This will call the saveAssetToFile function of the asset.
	AssetHandle<T> createAsset(const T& object, const FileHandler::CompletePath& assetPath)
	{
		AssetHandle<T> newHandle = allocate(object);
		newHandle->createNewAssetFile(assetPath);
		return newHandle;
	}

	// Same as createAsset but create the asset as a default asset which key is a name and not a path.
	// Catefull ! If a default asset with the same name already exists, it will be override !
	AssetHandle<T> createDefaultAsset(const T& object, const std::string& assetName)
	{
		AssetHandle<T> newHandle = allocate(object);
		defaults[assetName] = newHandle;
		return newHandle;
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
	Asset* loadAsset(const FileHandler::CompletePath& assetPath) override
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
			fileId.load(readMetaFile["assetId"]);
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

		return newAsset;
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
			outHandle = found->second;
			return true;
		}
		else
		{
			false;
		}
	}

	const AssetHandle<T>& getDefaultAsset(const std::string& assetName)
	{
		auto found = defaults.find(assetName);
		assert(found != defaults.end());
		return found->second;
	}

	bool getAsset(const AssetId& assetId, AssetHandle<T>& outHandle)
	{
		auto found = dataLinkMapping.find(assetId);
		if (found != dataLinkMapping.end())
		{
			outHandle = found->second;
			return true;
		}
		else
		{
			return false;
		}
	}

	const AssetHandle<T>& getAsset(const AssetId& assetId)
	{
		auto found = dataLinkMapping.find(assetId);
		assert(found != dataLinkMapping.end());
		AssetLinks* foundLinks = links[found->second];
		assert(foundLink != nullptr);
		T* foundData = datas[foundLinks->dataIdx];
		assert(foundData != nullptr);
		return AssetHandle<T>(foundData, foundLinks);
	}

	Asset* getAssetPtr(const AssetId& assetId) const override
	{
		auto found = dataLinkMapping.find(assetId);
		assert(found != dataLinkMapping.end());
		AssetLinks* foundLinks = links[found->second];
		assert(foundLink != nullptr);
		T* foundData = datas[foundLinks->dataIdx];
		assert(foundData != nullptr);
		return foundData;
	}

	bool assetExists(const AssetId& assetId) const override
	{
		return dataLinkMapping.find(assetId) != dataLinkMapping.end();
	}
};

class AssetLinks
{
public:
	int index;
	int dataIdx;
	AssetId id;
	std::vector<BaseAssetHandle*> handleRefs;

	static void swapLinks(AssetLinks* a, AssetLinks* b)
	{
		int c_dtaIdx = a->dataIdx;
		a->dataIdx = b->dataIdx;
		b->dataIdx = c_dtaIdx;

		std::vector<BaseAssetHandle*> c_links;
		c_links = a->handleRefs;
		a->handleRefs = b->handleRefs;
		b->handleRefs = c_links;

		AssetId c_assetId;
		c_assetId = a->id;
		a->id = b->id;
		b->id = c_assetId;
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
		handleRefs.clear();
	}

	int addLink(BaseAssetHandle* handleRef)
	{
		handleRefs.push_back(handleRef);
		return handleRefs.size();
	}

	void deleteLink(int linkIndex)
	{
		if (handleRefs.size() == 1)
		{
			handleRefs.pop_back();
		}
		else
		{
			std::iter_swap(handleRefs.begin() + linkIndex, handleRefs.end() - 1);
			handleRefs.pop_back();
			handleRefs[linkIndex]->linkIndex = linkIndex;
		}
	}

	void reset()
	{
		for (auto ref : handleRefs)
		{
			ref->reset();
		}

		handleRefs.clear();
	}

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(index, dataIdx, handleRefs);
	}
};

