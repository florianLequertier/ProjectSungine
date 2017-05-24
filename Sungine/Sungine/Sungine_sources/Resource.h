#pragma once

#include "FileHandler.h"
#include "IDrawableInInspector.h"
#include "IDGenerator.h"
#include "IResourcePtr.h"

class Scene;
class ResourceFactoryBase;

class Resource : public IDrawableInInspector
{
	friend ResourceFactoryBase;
	friend IResourcePtr;

protected:
	FileHandler::CompletePath m_completePath;
	std::string m_name;

	std::vector<IResourcePtr*> m_referencesToThis;
	bool m_isDefaultResource;
	ID m_resourceId;

public:
	Resource();
	virtual ~Resource();
	Resource(const FileHandler::CompletePath& completePath, bool isDefaultResource = false);

	// Load the resource at the given path, given the resource an id.
	virtual void init(const FileHandler::CompletePath& completePath, const ID& id);
	// Second step for loading. Will load pointers.
	virtual void resolvePointersLoading() = 0;
	// Save the resource. The resource should not be default resource and the complete path should be valid.
	virtual void save() = 0;

	const FileHandler::CompletePath& getCompletePath() const;
	const std::string& getName() const;
	bool getIsDefaultResource() const;
	ID getResourceID() const;

	virtual void drawInInspector(Scene & scene, const std::vector<IDrawableInInspector*>& selection) override;
	virtual void drawInInspector(Scene & scene) override;

	virtual void drawIconeInResourceTree();
	virtual void drawUIOnHovered();
	virtual void drawIconeInResourceField();
	virtual bool drawRightClicContextMenu(std::string& popupToOpen) { return false; }; // Nothing by default, only override by shder programs

	// TODO : CORE : Set these to privates : 
	void addReferenceToThis(IResourcePtr * newPtr);
	void removeReferenceToThis(IResourcePtr* deletedPtr);

private:
	void setName(const std::string& name);
	void setResourceID(const ID& id);
};