

#include "MotionState.h"


MotionState::MotionState(TransformNode* entityTransform)
{
	//auto transformMat = entityTransform->getModelMatrix();
	//m_physicTransform.setFromOpenGLMatrix(glm::value_ptr(transformMat));

	glm::quat rotation = entityTransform->getRotation();
	glm::vec3 translation = entityTransform->getTranslation();
	m_physicTransform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
	m_physicTransform.setOrigin(btVector3(translation.x, translation.y, translation.z));

	m_entityTransform = entityTransform;
}

MotionState::~MotionState()
{

}

void MotionState::getWorldTransform(btTransform& worldTransform) const
{
	worldTransform = m_physicTransform;
}

void MotionState::setWorldTransform(const btTransform& worldTransform)
{
	btQuaternion rotation = worldTransform.getRotation();
	btVector3 translation = worldTransform.getOrigin();

	//    m_entityTransform->setRotation(glm::quat( rotation.x(), rotation.y(), rotation.z(), rotation.w()));
	//    m_entityTransform->setTranslation(translation.x(), translation.y(), translation.z());
	m_physicTransform.setRotation(rotation);
	m_physicTransform.setOrigin(translation);

	//m_entityTransform->setTranslation(glm::vec3(translation.x(), translation.y(), translation.z()));
	//m_entityTransform->setRotation(glm::quat(rotation.w(), rotation.x(), rotation.y(), rotation.z()));
	m_entityTransform->applyTransformFromPhysicSimulation(glm::vec3(translation.x(), translation.y(), translation.z()), glm::quat(rotation.w(), rotation.x(), rotation.y(), rotation.z()));
}