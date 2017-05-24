#pragma once

#include <glm/glm.hpp>
#include "FrameBuffer.h"
#include "Texture.h"
#include "Mesh.h"
#include "Materials.h"
#include "ResourcePointer.h"
#include "RenderTarget.h"

class Entity;
class Editor;
class Scene;
namespace MVS {
	class NodeManager;
}

class Viewport
{
private:
	RenderTarget m_renderTarget;
	glm::vec2 m_position;
	bool m_isHovered;

public:
	Viewport()
		: m_renderTarget(std::make_shared<ColorAndDepthRTL>(), std::make_shared<ReflectionRTL>())
	{

	}

	const glm::vec2& getPosition() const
	{
		return m_position;
	}

	const glm::vec2& getSize() const
	{
		return m_renderTarget.getSize();
	}

	void setPosition(const glm::vec2& position)
	{
		m_position = position;
	}

	void setSize(const glm::vec2& size)
	{
		m_renderTarget.setSize(size);
	}

	void setIsHovered(bool state)
	{
		m_isHovered = state;
	}

	bool getIsHovered() const
	{
		return m_isHovered;
	}

	RenderTarget& getRenderTarget()
	{
		return m_renderTarget;
	}
};

/////////////////////////////////////////

class Inspector
{
private:
	char textValue[30];
	int intValue;
	float floatValue;
	glm::vec3 vector3Value;

	bool m_multipleEditing;
	Scene* m_currentScene;
	Editor* m_editorPtr;

public:
	Inspector(Editor* editorPtr);
	~Inspector();

	void setScene(Scene* currentScene);
	void drawUI();

	//void drawUI(const std::vector<Entity*>& entities);
	//void drawUI(const std::vector<PointLight*>& pointLights);
	//void drawUI(const std::vector<DirectionalLight*>& directionalLights);
	//void drawUI(const std::vector<SpotLight*>& spotLights);
	//void drawUI(const std::vector<MeshRenderer*>& meshRenderers);
	//void drawUI(const std::vector<Collider*>& colliders);
};

/////////////////////////////////////////

class SceneHierarchy
{
private:
	Scene* m_currentScene;
	Editor* m_editorPtr;
public:
	SceneHierarchy(Editor* editorPtr);
	void setScene(Scene* scene);
	Scene* getScene() const;
	void displayTreeEntityNode(Entity* entity, int &entityId, bool &setParenting, Entity*& parentToAttachSelected);
	void drawUI();
};

/////////////////////////////////////////

class WorldPropertiesTool
{
private:
	Scene* m_currentScene;

public:
	WorldPropertiesTool();
	void setScene(Scene* scene);
	Scene* getScene() const;
	void drawUI();
};

/////////////////////////////////////////

class DebugDrawRenderer
{
private:
	GlHelper::Framebuffer m_framebuffer;
	std::shared_ptr<Texture> m_texture;
	Mesh m_quadMesh;
	ResourcePtr<MaterialBlit> m_material;
	std::vector<std::string> m_outputNames;
	std::vector<int> m_separatorIndex;
	bool m_needSeparator;
	std::string m_currentOutputName;
	glm::vec2 m_frameSize;

public:
	DebugDrawRenderer();
	void drawTexture(GLuint textureId);
	void drawUI();
	void addSeparator();
	void setCurrentOutputName(const std::string& outputName);
	void drawOutputIfNeeded(const std::string& outputName, GLuint textureId);
	void setFrameSize(const glm::vec2& size);
};


