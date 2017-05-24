#pragma once

#include <assert.h>
#include <vector>

#include "ISingleton.h"
#include "ISerializable.h"

struct ID : public ISerializable
{
	int generation;
	int index;

	ID() : index(-1), generation(-1)
	{}

	ID(int _index, int _generation) : index(_index), generation(_generation)
	{}

	void reset()
	{
		index = -1;
		generation = -1;
	}

	bool operator==(const ID& other) const
	{
		return generation == other.generation && index == other.index;
	}

	bool operator<(const ID& other) const
	{
		return index < other.index;
	}

	virtual void save(Json::Value & entityRoot) const override
	{
		entityRoot["index"] = index;
		entityRoot["generation"] = generation;
	}

	virtual void load(const Json::Value & entityRoot) override
	{
		index = entityRoot.get("index", -1).asInt();
		generation = entityRoot.get("generation", -1).asInt();
	}

	bool isValid() const
	{
		return index != -1 && generation != -1;
	}
};

template<>
struct std::hash<ID>
{
	std::size_t operator()(const ID& key) const
	{
		return std::hash<int>()(key.index);
	}
};


template<typename T>
class IDGenerator : public ISingleton<IDGenerator<T>>
{
private:
	std::vector<bool> m_used;
	std::vector<int> m_generation;

public:
	IDGenerator()
	{
		m_used.resize(100, false);
		m_generation.resize(100, 0);
	}
	
	bool isIDLocked(const ID& id)
	{
		// Out of bounds
		if (m_used.size() <= id.index || m_generation.size() <= id.index)
			return false;

		// Generation mismatch
		return m_generation[id.index] == id.generation ? m_used[id.index] : false;
	}

	bool isIDValid(const ID& id)
	{
		// Invalid
		if (id.generation < 0 || id.index < 0)
			return false;

		return isIDLocked(id);
	}

	void lockID(ID idToLock)
	{
		if (m_used.size() <= idToLock.index)
		{
			const idCountToAdd = idToLock.index - m_used.size() +1;
			for (int i = 0; i < idCountToAdd; i++)
			{
				m_used.push_back(false);
				m_generation.push_back(0);
			}
		}

		assert(m_used[idToLock.index] == false); // ID must be free

		m_used[idToLock.index] = true;
		m_generation[idToLock.index] = idToLock.generation;	

		assert(m_used.size() == m_generation.size());
	}

	ID lockID()
	{
		auto& itFirstUnsused =  std::find(m_used.begin(), m_used.end(), false);
		if (itFirstUnsused != m_used.end())
		{
			int idFirstUnused = std::distance(m_used.begin(), itFirstUnsused);
			m_generation[idFirstUnused]++;
			*itFirstUnsused = true;
			return ID(idFirstUnused, m_generation[idFirstUnused]);
		}
		else
		{
			m_generation.push_back(0);
			m_used.push_back(true);
			assert(m_used.size() == m_generation.size());
			return ID(m_used.size() - 1, m_generation.back());
		}
	}

	void freeID(ID& id)
	{
		assert(m_used.size() > id.index && m_generation.size() > id.index);
		m_used[id.index] = false;
	}
};

