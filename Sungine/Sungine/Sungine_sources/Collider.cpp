

#include "Collider.h"
//forwards : 
#include "Ray.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"
#include "Rigidbody.h"

CollisionInfo::CollisionInfo(Rigidbody* _rigidbody, const glm::vec3& _point, const glm::vec3& _normal) : rigidbody(_rigidbody), point(_point), normal(_normal)
{ }

///////////////////////////////

Collider::Collider(Mesh* _visualMesh, MaterialInstance* _visualMaterial)
	: visualMesh(_visualMesh)
	, visualMaterial(_visualMaterial)
	, translation(0,0,0)
	, scale(1,1,1)
	, offsetPosition(0,0,0)
	, offsetScale(1,1,1)
	, origin(0,0,0)
{

}

Collider::~Collider()
{

}

void Collider::setVisual(ResourcePtr<Mesh> _visualMesh, ResourcePtr<MaterialInstance> _visualMaterial)
{
	visualMesh = _visualMesh;
	visualMaterial = _visualMaterial;
}

void Collider::applyTransform(const glm::vec3 & _translation, const glm::vec3 & _scale, const glm::quat & _rotation)
{
	this->rotation = _rotation;

	applyTransform(_translation, _scale);
}

void Collider::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	this->rotation = rotation;
	this->translation = translation;

	updateModelMatrix();
}

void Collider::applyTransform(const glm::vec3& _translation, const glm::vec3& _scale)
{
	translation = _translation;
	scale = _scale;

	updateModelMatrix();
}

//keep the translation, apply only the scale
void Collider::applyScale(const glm::vec3& _scale)
{
	scale = _scale;

	updateModelMatrix();
}

//keep the translation, append the scale 
void Collider::appendScale(const glm::vec3& _scale)
{
	scale *= _scale;

	updateModelMatrix();
}

//keep the scale, apply only the translation
void Collider::applyTranslation(const glm::vec3& _translation)
{
	translation = _translation;

	updateModelMatrix();
}

//keep the scale, append the translation 
void Collider::appendTranslation(const glm::vec3& _translation)
{
	translation += _translation;

	updateModelMatrix();
}

void Collider::applyRotation(const glm::quat & _rotation)
{
	rotation = _rotation;

	updateModelMatrix();
}

void Collider::appendRotation(const glm::quat & _rotation)
{
	rotation *= _rotation;

	updateModelMatrix();
}

void Collider::setOffsetPosition(glm::vec3 _offset)
{
	offsetPosition = _offset;
	updateOffsetMatrix();
	updateModelMatrix();
}

void Collider::setOffsetScale(glm::vec3 _offset)
{
	offsetScale = _offset;
	updateOffsetMatrix();
	updateModelMatrix();
}

void Collider::addOffsetPosition(glm::vec3 _offset)
{
	offsetPosition += _offset;
	updateOffsetMatrix();
	updateModelMatrix();
}

void Collider::addOffsetScale(glm::vec3 _offset)
{
	offsetScale += _offset;
	updateOffsetMatrix();
	updateModelMatrix();
}

void Collider::setOrigin(const glm::vec3 & _origin)
{
	origin = _origin;

	updateModelMatrix();
}

glm::vec3 Collider::getOrigin() const
{
	return origin;
}

glm::mat4 Collider::getModelMatrix()
{
	return modelMatrix;
}

glm::mat4 Collider::getOffsetMatrix() const
{
	return offsetMatrix;
}

void Collider::updateModelMatrix()
{
	modelMatrix = glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale) * offsetMatrix;
}

void Collider::updateOffsetMatrix()
{
	offsetMatrix = glm::translate(glm::mat4(1), offsetPosition) * glm::scale(glm::mat4(1), offsetScale);
	// TODO : Speed up this process
	Rigidbody* rigidbody = getComponent<Rigidbody>(Component::ComponentType::RIGIDBODY);
	if (rigidbody != nullptr) {
		rigidbody->makeShape();
	}
}

void Collider::render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& color)
{
	if (!visualMesh.isValid() || !visualMaterial.isValid())
	{
		PRINT_ERROR("visual mesh or visual material not valid when rendering collider.");
		return;
	}

	//glm::mat4 mvp = projection * view * modelMatrix;

	MaterialUnlit* unlitMat = static_cast<MaterialUnlit*>(visualMaterial.get()); //TODO : à modifier avec l'upgrade du pipeline de visualisation

	unlitMat->use();
	unlitMat->setUniformModelMatrix(modelMatrix);
	unlitMat->setUniformViewMatrix(view);
	unlitMat->setUniformProjectionMatrix(projection);
	//unlitMat->setUniform_MVP(mvp);
	//unlitMat->setUniform_normalMatrix(glm::mat4(1)); //no need normals

	unlitMat->setUniform_color(color);

	visualMesh->draw();
}

void Collider::save(Json::Value & rootComponent) const
{
	Component::save(rootComponent);
 
	if (visualMaterial.isValid())
		visualMaterial.save(rootComponent["visualMaterialName"]);
	if(visualMesh.isValid())
		visualMesh.save(rootComponent["visualMeshName"]);

	rootComponent["offsetPosition"] = toJsonValue(offsetPosition);
	rootComponent["offsetScale"] = toJsonValue(offsetScale);
	rootComponent["origin"] = toJsonValue(origin);
	rootComponent["translation"] = toJsonValue(translation);
	rootComponent["scale"] = toJsonValue(scale);
	rootComponent["rotation"] = toJsonValue(rotation);
	rootComponent["modelMatrix"] = toJsonValue(modelMatrix);
}

void Collider::load(const Json::Value & rootComponent)
{
	Component::load(rootComponent);

	if (visualMaterial.isValid())
		visualMaterial.load(rootComponent["visualMaterialName"]);
	if (visualMesh.isValid())
		visualMesh.load(rootComponent["visualMeshName"]);



	offsetPosition = fromJsonValue<glm::vec3>(rootComponent["offsetPosition"], glm::vec3());
	offsetScale = fromJsonValue<glm::vec3>(rootComponent["offsetScale"], glm::vec3());
	origin = fromJsonValue<glm::vec3>(rootComponent["origin"], glm::vec3());
	translation = fromJsonValue<glm::vec3>(rootComponent["translation"], glm::vec3());
	scale = fromJsonValue<glm::vec3>(rootComponent["scale"], glm::vec3());
	rotation = fromJsonValue<glm::quat>(rootComponent["rotation"], glm::quat());
	modelMatrix = fromJsonValue<glm::mat4>(rootComponent["modelMatrix"], glm::mat4());
}

void Collider::drawInInspector(Scene & scene)
{
	glm::vec3 tmpOffset = offsetPosition;
	if (ImGui::InputFloat3("offset position", &tmpOffset[0]))
		setOffsetPosition(tmpOffset);
}

void Collider::drawInInspector(Scene & scene, const std::vector<Component*>& components)
{
	if (components.size() == 0)
		return;

	glm::vec3 tmpOffset = offsetPosition; //we are already in the first component
	if (ImGui::InputFloat3("offset position", &tmpOffset[0]))
	{
		for (auto component : components)
		{
			Collider* castedComponent = static_cast<Collider*>(component);
			castedComponent->setOffsetPosition(tmpOffset);
		}
	}
}

void Collider::onAfterComponentAddedToEntity(Entity& entity)
{
	Rigidbody* rigidbody = entity.getComponent<Rigidbody>(Component::ComponentType::RIGIDBODY);
	if (rigidbody != nullptr)
		rigidbody->makeShape(); //order the ridigbody to reupdate it collider shape
	entity.applyTransform();
}

void Collider::onAfterComponentErasedFromEntity(Entity& entity)
{
	Rigidbody* rigidbody = entity.getComponent<Rigidbody>(Component::ComponentType::RIGIDBODY);
	if (rigidbody != nullptr)
		rigidbody->makeShape(); //order the ridigbody to reupdate it collider shape
}


///////////////////////////////////////////

COMPONENT_IMPLEMENTATION_CPP(BoxCollider)

BoxCollider::BoxCollider(Mesh* _visualMesh, MaterialInstance* _visualMaterial)
	: Collider(BOX_COLLIDER, getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"))
{
	localTopRight = glm::vec3(0.5f, 0.5f, 0.5f);
	localBottomLeft = glm::vec3(-0.5f, -0.5f, -0.5f);

	topRight = localTopRight;
	bottomLeft = localBottomLeft;
}

void BoxCollider::updateModelMatrix()
{
	Collider::updateModelMatrix();
	//modelMatrix = glm::translate(glm::mat4(1), offsetPosition + translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale * offsetScale) * glm::translate(glm::mat4(1), -origin);

	topRight = glm::vec3( modelMatrix * glm::vec4(localTopRight, 1) );
	bottomLeft = glm::vec3( modelMatrix * glm::vec4(localBottomLeft, 1) );
}

void BoxCollider::render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& color)
{
	Collider::render(projection, view, color);
}

void BoxCollider::debugLog()
{
	std::cout << "collider bottomLeft : " << bottomLeft.x << ", " << bottomLeft.y << ", " << bottomLeft.z << std::endl;
	std::cout << "collider topRight : " << topRight.x << ", " << topRight.y << ", " << topRight.z << std::endl;
}

bool BoxCollider::isIntersectedByRay(const Ray& ray, float* t)
{
	glm::vec3 dir = ray.getDirection();
	glm::vec3 orig = ray.getOrigin();
	/*
	glm::vec3 H[3];
	H[0] = rotation*glm::vec3(1.f, 0.f, 0.f);
	H[1] = rotation*glm::vec3(0.f, 1.f, 0.f);
	H[2] = rotation*glm::vec3(0.f, 0.f, 1.f);
	bool test = rayOBBoxIntersect(orig, dir, translation, H, scale, t);
	return test;
	*/
	
	// r.dir is unit direction vector of ray
	glm::vec3 dirfrac;
	dirfrac.x = 1.0f / dir.x;
	dirfrac.y = 1.0f / dir.y;
	dirfrac.z = 1.0f / dir.z;
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
	float t1 = (bottomLeft.x - orig.x)*dirfrac.x;
	float t2 = (topRight.x - orig.x)*dirfrac.x;
	float t3 = (bottomLeft.y - orig.y)*dirfrac.y;
	float t4 = (topRight.y - orig.y)*dirfrac.y;
	float t5 = (bottomLeft.z - orig.z)*dirfrac.z;
	float t6 = (topRight.z - orig.z)*dirfrac.z;

	float tmin = glm::max<float>(glm::max<float>(glm::min<float>(t1, t2), glm::min<float>(t3, t4)), glm::min<float>(t5, t6));
	float tmax = glm::min<float>(glm::min<float>(glm::max<float>(t1, t2), glm::max<float>(t3, t4)), glm::max<float>(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if (tmax < 0)
	{
		if(t!=nullptr)
			*t = tmax;

		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		if (t != nullptr)
			*t = tmax;

		return false;
	}

	if (t != nullptr)
		*t = tmin;

	if (tmin > ray.getLength())
		return false;

	return true;
	

	/*


	glm::vec3 t;
	int maxIndex = 0;
	for (int i = 0; i < 3; i++)
	{
	if (direction[i] > 0)
	t[i] = (other.bottomLeft[i] - origin[i]) / direction[i];
	else
	t[i] = (other.topRight[i] - origin[i]) / direction[i];

	if (t[i] > t[maxIndex])
	{
	maxIndex = i;
	}
	}

	if (t[maxIndex] >= 0 && t[maxIndex] < length)
	{
	glm::vec3 pt = at(t[maxIndex]);

	int o1 = (maxIndex + 1) % 3;
	int o2 = (maxIndex + 2) % 3;

	return((pt[o1] > other.bottomLeft[o1] && pt[o1] < other.topRight[o1]) &&
	(pt[o2] > other.bottomLeft[o2] && pt[o2] < other.topRight[o2]));
	}

	return false;*/
}

void BoxCollider::drawInInspector(Scene& scene)
{
	Collider::drawInInspector(scene);
	glm::vec3 tmpOffsetScale = offsetScale;
	if (ImGui::InputFloat3("offset scale", &tmpOffsetScale[0]))
		setOffsetScale(tmpOffsetScale);
}

void BoxCollider::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	Collider::drawInInspector(scene, components);
	glm::vec3 tmpOffsetScale = offsetScale;
	if (ImGui::InputFloat3("offset scale", &tmpOffsetScale[0]))
	{
		for (auto component : components)
		{
			BoxCollider* castedComponent = static_cast<BoxCollider*>(component);

			setOffsetScale(tmpOffsetScale);
		}
	}
}

void BoxCollider::coverMesh(const Mesh& mesh)
{
	origin = mesh.origin;
	glm::vec3 dimensions = (mesh.topRight - mesh.bottomLeft)*scale;

	offsetScale = dimensions;
	offsetPosition = dimensions * 0.5f + origin*dimensions + mesh.bottomLeft*scale;// -translation;

	updateOffsetMatrix();
	updateModelMatrix();
}

void BoxCollider::cover(glm::vec3 min, glm::vec3 max, glm::vec3 _origin)
{
	origin = _origin;
	glm::vec3 dimensions = (min - max);

	offsetScale = dimensions;
	offsetPosition = dimensions * 0.5f + origin*dimensions + min;// -translation;

	updateOffsetMatrix();
	updateModelMatrix();
}

btCollisionShape * BoxCollider::makeShape()
{
	return new btBoxShape(btVector3(0.5f*offsetScale.x, 0.5f*offsetScale.y, 0.5f*offsetScale.z));
}

void BoxCollider::save(Json::Value & rootComponent) const
{
	Collider::save(rootComponent);

	rootComponent["localTopRight"] = toJsonValue(localTopRight);
	rootComponent["localBottomLeft"] = toJsonValue(localBottomLeft);
	rootComponent["topRight"] = toJsonValue(topRight);
	rootComponent["bottomLeft"] = toJsonValue(bottomLeft);
}

void BoxCollider::load(const Json::Value & rootComponent)
{
	Collider::load(rootComponent);

	localTopRight = fromJsonValue<glm::vec3>(rootComponent["localTopRight"], glm::vec3());
	localBottomLeft = fromJsonValue<glm::vec3>(rootComponent["localBottomLeft"], glm::vec3());
	topRight = fromJsonValue<glm::vec3>(rootComponent["topRight"], glm::vec3());
	bottomLeft = fromJsonValue<glm::vec3>(rootComponent["bottomLeft"], glm::vec3());
}

//////////////////////////////////////////////

COMPONENT_IMPLEMENTATION_CPP(CapsuleCollider)

CapsuleCollider::CapsuleCollider()
	: Collider(CAPSULE_COLLIDER, getMeshFactory().getDefault("capsuleWireframe"), getMaterialFactory().getDefault("wireframe"))
	, radius(0.5f)
	, height(2.f)
{

}

void CapsuleCollider::render(const glm::mat4 & projection, const glm::mat4 & view, const glm::vec3 & color)
{
	Collider::render(projection, view, color);
}

void CapsuleCollider::debugLog()
{
	//nothing
}

bool CapsuleCollider::isIntersectedByRay(const Ray & ray, float * t)
{
	//nothing
	return false;
}

void CapsuleCollider::drawInInspector(Scene & scene)
{
	Collider::drawInInspector(scene);
	if (ImGui::InputFloat("height", &height)) {
		offsetScale.y = height*0.5f;
		offsetScale.x = radius * 2.f;
		offsetScale.z = radius * 2.f;
		updateOffsetMatrix();
	}
	if (ImGui::InputFloat("radius", &radius)) {
		offsetScale.y = height*0.5f;
		offsetScale.x = radius * 2.f;
		offsetScale.z = radius * 2.f;
		updateOffsetMatrix();
	}
}

void CapsuleCollider::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	Collider::drawInInspector(scene, components);

	if (ImGui::InputFloat("height", &height)) 
	{
		for (auto component : components)
		{
			CapsuleCollider* castedComponent = static_cast<CapsuleCollider*>(component);

			castedComponent->offsetScale.y = height*0.5f;
			castedComponent->offsetScale.x = radius * 2.f;
			castedComponent->offsetScale.z = radius * 2.f;
			castedComponent->updateOffsetMatrix();
		}
	}
	if (ImGui::InputFloat("radius", &radius)) 
	{
		for (auto component : components)
		{
			CapsuleCollider* castedComponent = static_cast<CapsuleCollider*>(component);

			castedComponent->offsetScale.y = height*0.5f;
			castedComponent->offsetScale.x = radius * 2.f;
			castedComponent->offsetScale.z = radius * 2.f;
			castedComponent->updateOffsetMatrix();
		}
	}
}

void CapsuleCollider::coverMesh(const Mesh & mesh)
{
	origin = mesh.origin;
	glm::vec3 dimensions = (mesh.topRight - mesh.bottomLeft)*scale;

	offsetScale = dimensions;
	offsetPosition = dimensions * 0.5f + origin*dimensions + mesh.bottomLeft*scale;// -translation;

	updateOffsetMatrix();
	updateModelMatrix();
}

void CapsuleCollider::cover(glm::vec3 min, glm::vec3 max, glm::vec3 _origin)
{
	origin = _origin;
	glm::vec3 dimensions = (min - max)*scale;

	offsetScale = dimensions;
	offsetPosition = dimensions * 0.5f + origin*dimensions + min*scale;// -translation;

	height = offsetScale.y * 2.f;
	radius = offsetScale.x*0.5f;

	updateOffsetMatrix();
	updateModelMatrix();
}

btCollisionShape * CapsuleCollider::makeShape()
{
	return new btCapsuleShape(0.5f*offsetScale.x, 2.f*offsetScale.y);
}

void CapsuleCollider::save(Json::Value & rootComponent) const
{
	Collider::save(rootComponent);
	rootComponent["height"] = height;
	rootComponent["radius"] = radius;
}

void CapsuleCollider::load(const Json::Value & rootComponent)
{
	Collider::load(rootComponent);
	height = rootComponent.get("height", 1).asFloat();
	radius = rootComponent.get("radius", 1).asFloat();
}
