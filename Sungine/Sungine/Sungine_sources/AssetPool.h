#pragma once

#include "ObjectPool.h"

class IAssetPool
{

};

template<typename T>
class AssetPool<T> : public ObjectPool<T>, IAssetPool
{
private:
	std::map<std::string, ID> m_mappingPathID;
	std::map<ID, Handler<T>> m_mappingIDAsset;

public:
	void loadAsset(const std::string& path)
	{
		// Si on trouve le path dans mappingPath ID, on met l'ID à l'asset, sinon on créé un nouvel ID
		// on alloue un nouvel asset et on lui file le nouvel ID
		// On créé une archive avec le path
		// On appel la methode de load sur le nouvel asset en lui filant l'archive créée
	}

	void createAsset(T& object, const std::string& path)
	{
		// On verifie qu'il n'y a rien à path,
		// on créé un nouvel ID
		// on alloue un nouvel asset et on lui file le nouvel ID
	}

	bool deleteAsset(const std::string& path);
	bool deleteAsset(ID id);
	bool containsPath(const std::string& path);
	bool containsID(ID id);
	Handler<T> getAsset(const std::string& path);
	Handler<T> getAsset(ID id);

};