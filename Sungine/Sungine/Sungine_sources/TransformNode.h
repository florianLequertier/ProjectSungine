#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "jsoncpp/json/json.h"

#include "Utils.h"
#include "ISerializable.h"
#include "IDrawableInInspector.h"

#include "Object.h"

class Entity; //forward

class TransformNode : public Object
{
protected:
	glm::vec3 m_translation;
	glm::vec3 m_scale;
	glm::quat m_rotation;
	glm::vec3 m_eulerRotation;

	glm::vec3 m_localTranslation;
	glm::vec3 m_localScale;
	glm::quat m_localRotation;
	glm::vec3 m_localEulerRotation;

	glm::vec3 m_parentTranslation;
	glm::vec3 m_parentScale;
	glm::quat m_parentRotation;

	glm::mat4 m_modelMatrix;
public:
	TransformNode();
	virtual ~TransformNode();

	glm::mat4 getModelMatrix();

	glm::vec3 getTranslation();
	glm::vec3 getScale();
	glm::quat getRotation();
	glm::vec3 getEulerRotation();

	glm::vec3 getLocalTranslation();
	glm::vec3 getLocalScale();
	glm::quat getLocalRotation();
	glm::vec3 getLocalEulerRotation();

	void translate(glm::vec3 const& t);
	void setTranslation(glm::vec3 const& t);
	void scale(glm::vec3 const& s);
	void setScale(glm::vec3 const& s);
	void rotate(glm::quat const& q);
	void setRotation(glm::quat const& q);
	void setEulerRotation(glm::vec3 const& q);

	void localTranslate(glm::vec3 const& t);
	void setLocalTranslation(glm::vec3 const& t);
	void localScale(glm::vec3 const& s);
	void setLocalScale(glm::vec3 const& s);
	void localRotate(glm::quat const& q);
	void setLocalRotation(glm::quat const& q);
	void setLocalEulerRotation(glm::vec3 const& q);

	void setParentTransform(const glm::vec3& parentTranslation = glm::vec3(0, 0, 0), const glm::vec3& parentScale = glm::vec3(1, 1, 1), const glm::quat& parentRotation = glm::quat());
	void setParentTransform(const glm::vec3& parentTranslation, const glm::quat& parentRotation);
	void setParentTransform(const TransformNode& parentTransform);

	void updateModelMatrix();

	virtual void onChangeModelMatrix() = 0;

	//apply transform on this entity, and apply transform on all its components.
	virtual void applyTransform() = 0;
	//function to apply transform to all children.
	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) = 0;
	//function to apply transform to all children, but this one don't scale the object.
	virtual void applyTransform(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) = 0;
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) = 0;

	//void drawUI(bool local = false);
	//void drawInInspector(bool local, const std::vector<IDrawableInInspector*>& selection);

	//virtual void save(Json::Value& entityRoot) const override;
	//virtual void load(const Json::Value& entityRoot) override;

};

