#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Materials.h"
#include "Texture.h"
#include "BatchableWith.h"
#include "IRenderableComponent.h"
#include "IDrawable.h"

class Billboard : public Component, public IRenderableComponent, public IBatchableWith<MaterialBillboard>
{
	REFLEXION_HEADER(Billboard)
	COMPONENT_IMPLEMENTATION_HEADER(Billboard)

private:
	glm::vec3 m_translation;
	glm::vec2 m_scale;
	ResourcePtr<Mesh> m_quadMesh;
	ResourcePtr<Material> m_billboardMaterial;
	ResourcePtr<Texture> m_texture;
	//std::string m_textureName;
	glm::vec4 m_color;

public:
	Billboard();
	~Billboard();

	void render(const glm::mat4& projection, const glm::mat4& view);

	void setTranslation(const glm::vec3& translation);
	void setScale(const glm::vec2& scale);
	void setColor(const glm::vec4& color);
	glm::vec3 getTranslation() const;
	glm::vec2 getScale() const;
	glm::vec4 getColor() const;
	const Texture& getTexture() const;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
	
	virtual void drawInInspector(Scene & scene) override;
	virtual void drawInInspector(Scene & scene, const std::vector<Component*>& components) override;

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

	// Herited from IRenderableComponent
	virtual const IDrawable & getDrawable(int drawableIndex) const override;
	virtual const Material & getDrawableMaterial(int drawableIndex) const override;
	virtual const int getDrawableCount() const override;
	virtual Component* getAsComponent() override;

	// Herited from IDrawable
	virtual const AABB & getVisualBoundingBox() const override;
	virtual void draw() const override;
	virtual const glm::mat4& getModelMatrix() const override;
	virtual bool castShadows() const override;
	virtual const Billboard* getAsBillboardIfPossible() const override;

	virtual void setExternalsOf(const MaterialBillboard& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const;

	virtual void onAfterComponentAddedToScene(Scene & scene) override;
	virtual void onBeforeComponentErasedFromScene(Scene & scene) override;
};

REFLEXION_CPP(Billboard)
REFLEXION_InheritFrom(Billboard, Component)
