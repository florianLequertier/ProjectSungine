#include "Resource.h"
#include "Project.h"
#include "Scene.h"

Resource::Resource()
	: m_isDefaultResource(true)
{
	m_resourceId = IDGenerator<Resource>::instance().lockID();
}

Resource::~Resource()
{
	if (m_resourceId.isValid())
		IDGenerator<Resource>::instance().freeID(m_resourceId);

	for (auto reference : m_referencesToThis)
	{
		reference->reset();
	}
}

Resource::Resource(const FileHandler::CompletePath& completePath, bool isDefaultResource)
	: m_completePath(completePath)
	, m_name(m_completePath.getFilename())
	, m_isDefaultResource(isDefaultResource)
{
	m_resourceId = IDGenerator<Resource>::instance().lockID();
}

void Resource::init(const FileHandler::CompletePath& completePath, const ID& id)
{
	if(m_resourceId.isValid())
		IDGenerator<Resource>::instance().freeID(m_resourceId); // Free old ID

	assert(IDGenerator<Resource>::instance().isIDLocked(id)); // The given id must be locked
	m_resourceId = id;
	m_isDefaultResource = false;
	m_completePath = completePath;
	m_name = m_completePath.getFilename();
	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path shouldn't be absolute
}

const FileHandler::CompletePath& Resource::getCompletePath() const
{
	return m_completePath;
}

const std::string& Resource::getName() const
{
	return m_name;
}

bool Resource::getIsDefaultResource() const
{
	return m_isDefaultResource;
}

ID Resource::getResourceID() const
{
	return m_resourceId;
}

void Resource::setName(const std::string & name)
{
	m_name = name;
}

void Resource::setResourceID(const ID & id)
{
	m_resourceId = id;
}

void Resource::addReferenceToThis(IResourcePtr * newPtr)
{
	m_referencesToThis.push_back(newPtr);
}

void Resource::removeReferenceToThis(IResourcePtr * deletedPtr)
{
	auto& found = std::find(m_referencesToThis.begin(), m_referencesToThis.end(), deletedPtr);
	assert(found != m_referencesToThis.end());
	if (found != m_referencesToThis.end())
	{
		std::iter_swap(found, m_referencesToThis.end() - 1);
		m_referencesToThis.pop_back();
	}
}

void Resource::drawInInspector(Scene & scene, const std::vector<IDrawableInInspector*>& selection)
{
	assert(false && "multiple edition of resource isn't implemented.");
}

void Resource::drawInInspector(Scene & scene)
{
	ImGui::Text(getName().c_str());
}

void Resource::drawIconeInResourceTree()
{
	ImGui::Dummy(ImVec2(1, 1));
}

void Resource::drawUIOnHovered()
{
	// Nothing
}

void Resource::drawIconeInResourceField()
{
	ImGui::Dummy(ImVec2(1, 1));
}
