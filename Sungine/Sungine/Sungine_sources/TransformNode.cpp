

#include "TransformNode.h"
#include "Entity.h" //forward


TransformNode::TransformNode() : m_translation(0, 0, 0), m_scale(1, 1, 1), m_eulerRotation(0, 0, 0), m_localEulerRotation(0, 0, 0), m_localScale(1, 1, 1), m_localTranslation(0, 0, 0)
{
	//init parent transform to identity
	setParentTransform();
}

TransformNode::~TransformNode()
{

}

glm::mat4 TransformNode::getModelMatrix()
{
	return m_modelMatrix;
}

glm::vec3 TransformNode::getTranslation()
{
	return m_translation;
}

glm::vec3 TransformNode::getScale()
{
	return m_scale;
}

glm::quat TransformNode::getRotation()
{
	return m_rotation;
}

glm::vec3 TransformNode::getEulerRotation()
{
	return m_eulerRotation;
}

glm::vec3 TransformNode::getLocalTranslation()
{
	return m_localTranslation;
}

glm::vec3 TransformNode::getLocalScale()
{
	return m_localScale;
}

glm::quat TransformNode::getLocalRotation()
{
	return m_localRotation;
}

glm::vec3 TransformNode::getLocalEulerRotation()
{
	return m_localEulerRotation;
}

void TransformNode::translate(glm::vec3 const& t)
{
	m_translation += t;
	m_localTranslation += t;

	updateModelMatrix();
}

void TransformNode::setTranslation(glm::vec3 const& t)
{
	m_translation = t;
	m_localTranslation = t;

	updateModelMatrix();
}

void TransformNode::scale(glm::vec3 const& s)
{
	m_scale *= s;
	m_localScale *= s;

	updateModelMatrix();
}

void TransformNode::setScale(glm::vec3 const& s)
{
	m_scale = s;
	m_localScale = s;

	updateModelMatrix();
}

void TransformNode::rotate(glm::quat const& q)
{
	m_rotation *= q;
	m_localRotation *= q;

	updateModelMatrix();
}

void TransformNode::setRotation(glm::quat const& q)
{
	m_rotation = q;
	m_localRotation = q;

	updateModelMatrix();
}

void TransformNode::setEulerRotation(glm::vec3 const & q)
{
	m_eulerRotation = q;
	m_localEulerRotation = q;
	setRotation(glm::quat(m_eulerRotation));

	updateModelMatrix();
}

void TransformNode::localTranslate(glm::vec3 const & t)
{
	m_localTranslation += t;

	updateModelMatrix();
}

void TransformNode::setLocalTranslation(glm::vec3 const & t)
{
	m_localTranslation = t;

	updateModelMatrix();
}

void TransformNode::localScale(glm::vec3 const & s)
{
	m_localScale += s;

	updateModelMatrix();
}

void TransformNode::setLocalScale(glm::vec3 const & s)
{
	m_localScale = s;

	updateModelMatrix();
}

void TransformNode::localRotate(glm::quat const & q)
{
	m_localRotation *= q;

	updateModelMatrix();
}

void TransformNode::setLocalRotation(glm::quat const & q)
{
	m_localRotation = q;

	updateModelMatrix();
}

void TransformNode::setLocalEulerRotation(glm::vec3 const & q)
{
	m_localEulerRotation = q;
	setLocalRotation(glm::quat(m_localEulerRotation));
}

void TransformNode::setParentTransform(const glm::vec3& parentTranslation, const glm::vec3& parentScale, const glm::quat& parentRotation)
{
	m_parentRotation = parentRotation;
	m_parentTranslation = parentTranslation;
	m_parentScale = parentScale;

	m_localTranslation -= m_parentTranslation;
}

void TransformNode::setParentTransform(const glm::vec3 & parentTranslation, const glm::quat & parentRotation)
{
	m_parentRotation = parentRotation;
	m_parentTranslation = parentTranslation;

	m_localTranslation -= m_parentTranslation;
}

void TransformNode::setParentTransform(const TransformNode & parentTransform)
{
	setParentTransform(m_translation, m_scale, m_rotation);
}

void TransformNode::updateModelMatrix()
{
	m_translation = m_parentRotation * m_localTranslation + m_parentTranslation;
	m_scale = m_localScale * m_parentScale;
	m_rotation = m_localRotation * m_parentRotation;

	m_modelMatrix = glm::translate(glm::mat4(1), m_translation) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4(1), m_scale);

	onChangeModelMatrix();
}

void TransformNode::drawUI(bool local)
{

	glm::vec3 tmpTrans = m_translation;
	if (ImGui::InputFloat3("translation", &tmpTrans[0]))
	{
		setTranslation(tmpTrans);
		applyTransform();
	}
	if (!local)
	{
		glm::vec3 tmpRot = m_eulerRotation * (180.f / glm::pi<float>());
		if (ImGui::SliderFloat3("rotation", &tmpRot[0], 0, 360))
		{
			//m_eulerRotation = tmpRot * glm::pi<float>() / 180.f;
			//setRotation(glm::quat(m_eulerRotation));

			setEulerRotation(tmpRot * glm::pi<float>() / 180.f);
			applyTransform();
		}

		glm::vec3 tmpScale = m_scale;
		if (ImGui::InputFloat3("scale", &tmpScale[0]))
		{
			setScale(tmpScale);
			applyTransform();
		}
	}
	else
	{
		glm::vec3 tmpRot = m_localEulerRotation * (180.f / glm::pi<float>());
		if (ImGui::SliderFloat3("rotation", &tmpRot[0], 0, 360))
		{
			setLocalEulerRotation(tmpRot * glm::pi<float>() / 180.f);
			applyTransform();
		}

		glm::vec3 tmpScale = m_localScale;
		if (ImGui::InputFloat3("scale", &tmpScale[0]))
		{
			setLocalScale(tmpScale);
			applyTransform();
		}
	}
}

void TransformNode::drawInInspector(bool local, const std::vector<IDrawableInInspector*>& selection)
{
	if (selection.size() == 0)
		return;

	glm::vec3 tmpTrans = m_translation;
	if (ImGui::InputFloat3("translation", &tmpTrans[0]))
	{
		for (auto& node : selection)
		{
			Entity* entity = static_cast<Entity*>(node);
			entity->setTranslation(tmpTrans);
			entity->applyTransform();
		}
	}
	if (!local)
	{
		glm::vec3 tmpRot = m_eulerRotation * (180.f / glm::pi<float>());
		if (ImGui::SliderFloat3("rotation", &tmpRot[0], 0, 360))
		{
			for (auto& node : selection)
			{
				Entity* entity = static_cast<Entity*>(node);
				entity->setEulerRotation(tmpRot * glm::pi<float>() / 180.f);
				entity->applyTransform();
			}
		}

		glm::vec3 tmpScale = m_scale;
		if (ImGui::InputFloat3("scale", &tmpScale[0]))
		{
			for (auto& node : selection)
			{
				Entity* entity = static_cast<Entity*>(node);
				entity->setScale(tmpScale);
				entity->applyTransform();
			}
		}
	}
	else
	{
		glm::vec3 tmpRot = m_localEulerRotation * (180.f / glm::pi<float>());
		if (ImGui::SliderFloat3("rotation", &tmpRot[0], 0, 360))
		{
			for (auto& node : selection)
			{
				Entity* entity = static_cast<Entity*>(node);
				entity->setLocalEulerRotation(tmpRot * glm::pi<float>() / 180.f);
				entity->applyTransform();
			}
		}

		glm::vec3 tmpScale = m_localScale;
		if (ImGui::InputFloat3("scale", &tmpScale[0]))
		{
			for (auto& node : selection)
			{
				Entity* entity = static_cast<Entity*>(node);
				entity->setLocalScale(tmpScale);
				entity->applyTransform();
			}
		}
	}
}

void TransformNode::save(Json::Value & entityRoot) const
{
	entityRoot["translation"] = toJsonValue(m_translation);
	entityRoot["scale"] = toJsonValue(m_scale);
	entityRoot["rotation"] = toJsonValue(m_rotation);
	entityRoot["eulerRotation"] = toJsonValue(m_eulerRotation);

	entityRoot["localTranslation"] = toJsonValue(m_localTranslation);
	entityRoot["localScale"] = toJsonValue(m_localScale);
	entityRoot["localRotation"] = toJsonValue(m_localRotation);
	entityRoot["localEulerRotation"] = toJsonValue(m_localEulerRotation);

	entityRoot["parentTranslation"] = toJsonValue(m_parentTranslation);
	entityRoot["parentScale"] = toJsonValue(m_parentScale);
	entityRoot["parentRotation"] = toJsonValue(m_parentRotation);
	
	entityRoot["modelMatrix"] = toJsonValue(m_modelMatrix);
}

void TransformNode::load(const Json::Value & entityRoot)
{
	m_translation = fromJsonValue<glm::vec3>(entityRoot["translation"], glm::vec3());
	m_scale = fromJsonValue<glm::vec3>(entityRoot["scale"], glm::vec3());
	m_rotation = fromJsonValue<glm::quat>(entityRoot["rotation"], glm::quat());
	m_eulerRotation = fromJsonValue<glm::vec3>(entityRoot["eulerRotation"], glm::vec3());

	m_localTranslation = fromJsonValue<glm::vec3>(entityRoot["localTranslation"], glm::vec3());
	m_localScale = fromJsonValue<glm::vec3>(entityRoot["localScale"], glm::vec3());
	m_localRotation = fromJsonValue<glm::quat>(entityRoot["localRotation"], glm::quat());
	m_localEulerRotation = fromJsonValue<glm::vec3>(entityRoot["localEulerRotation"], glm::vec3());

	m_parentTranslation = fromJsonValue<glm::vec3>(entityRoot["parentTranslation"], glm::vec3());
	m_parentScale = fromJsonValue<glm::vec3>(entityRoot["parentScale"], glm::vec3());
	m_parentRotation = fromJsonValue<glm::quat>(entityRoot["parentRotation"], glm::quat());

	m_modelMatrix = fromJsonValue<glm::mat4>(entityRoot["modelMatrix"], glm::mat4());
}
