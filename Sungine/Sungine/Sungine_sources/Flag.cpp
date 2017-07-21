

#include "Flag.h"
//forwards : 
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"
#include "OctreeDrawer.h"
#include "EditorGUI.h"

namespace Physic {

	COMPONENT_IMPLEMENTATION_CPP(Flag)

	Flag::Flag() : Flag(getMaterialFactory().getDefault("defaultLit"))
	{

	}

	Flag::Flag(ResourcePtr<MaterialInstance> material, int subdivision, float width, float height) 
		: Component(FLAG)
		, m_mesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES | Mesh::USE_UVS | Mesh::USE_NORMALS | Mesh::USE_TANGENTS), 3, GL_STREAM_DRAW)
		, m_material(material)
		, m_subdivision(subdivision)
		, m_width(width)
		, m_height(height)
		, translation(0,0,0)
		, scale(1,1,1)
		, m_mass(0.1f)
		, m_rigidity(0.03f)
		, m_viscosity(0.003f)
		, m_autoCollisionDistance(0.01f)
		, m_autoCollisionRigidity(0.01f)
		, m_autoCollisionViscosity(0.001f)
		, m_materialName("default")
		, m_computeAutoCollision(false)
		, m_castShadows(true)
	{
		modelMatrix = glm::mat4(1);

		m_materialName = m_material->getName();

		//don't forget to change the origin to have the right pivot rotation
		origin = glm::vec3(-0.5f, -0.5f, 0.f);
		m_mesh.origin = glm::vec3(-0.5f, -0.5f, 0.f);

		m_mesh.topRight = glm::vec3(m_width, m_height, 0.f);
		m_mesh.bottomLeft = glm::vec3(0.f, 0.f, 0.f);

		m_mesh.vertices.clear();
		m_mesh.normals.clear();
		m_mesh.uvs.clear();
		m_mesh.triangleIndex.clear();
		m_mesh.tangents.clear();

		localPointPositions.clear();
		pointContainer.clear();
		linkShape.clear();
		linkShearing.clear();
		linkBlending.clear();

		generatePoints();
		generateMesh();

		m_mesh.initGl();

		initialyzePhysic();

		//cover the mesh with collider : 
		if (m_entity != nullptr)
		{
			auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
			if (collider != nullptr)
			{
				collider->coverMesh(m_mesh);
				collider->setOffsetScale(glm::vec3(1.f, 1.f, 2.f));
			}
		}

	}

	Flag::Flag(const Flag& other) 
		: Component(FLAG)
		, m_mesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES | Mesh::USE_UVS | Mesh::USE_NORMALS | Mesh::USE_TANGENTS), 3, GL_STREAM_DRAW)
	{
		m_material = other.m_material;
		m_subdivision = other.m_subdivision;
		m_width = other.m_width;
		m_height = other.m_height;
		translation = other.translation;
		scale = other.scale;
		m_mass = other.m_mass;
		m_rigidity = other.m_rigidity;
		m_viscosity = other.m_viscosity;
		m_materialName = other.m_materialName;
		m_autoCollisionDistance = other.m_autoCollisionDistance;
		m_computeAutoCollision = other.m_computeAutoCollision;
		m_autoCollisionRigidity = other.m_autoCollisionRigidity;
		m_autoCollisionViscosity = other.m_autoCollisionViscosity;
		m_castShadows = other.m_castShadows;


		modelMatrix = other.modelMatrix;

		//don't forget to change the origin to have the right pivot rotation
		origin = other.origin;
		m_mesh.origin = other.m_mesh.origin;

		m_mesh.topRight = other.m_mesh.topRight;
		m_mesh.bottomLeft = other.m_mesh.bottomLeft;

		m_mesh.vertices.clear();
		m_mesh.normals.clear();
		m_mesh.uvs.clear();
		m_mesh.triangleIndex.clear();
		m_mesh.tangents.clear();

		localPointPositions.clear();
		pointContainer.clear();
		linkShape.clear();
		linkShearing.clear();
		linkBlending.clear();

		generatePoints();
		generateMesh();

		m_mesh.initGl();

		initialyzePhysic();

		//cover the mesh with collider : 
		if (m_entity != nullptr)
		{
			auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
			if (collider != nullptr)
			{
				collider->coverMesh(m_mesh);
				collider->setOffsetScale(glm::vec3(1.f, 1.f, 2.f));
			}
		}
	}

	Flag & Flag::operator=(const Flag& other)
	{
		Component::operator=(other);

		m_mesh.freeGl();
		m_mesh.primitiveType = GL_TRIANGLES;
		m_mesh.vbo_usage = (Mesh::USE_INDEX | Mesh::USE_VERTICES | Mesh::USE_UVS | Mesh::USE_NORMALS | Mesh::USE_TANGENTS);
		m_mesh.coordCountByVertex = 3;
		m_mesh.drawUsage = GL_STREAM_DRAW;


		m_material = other.m_material;
		m_subdivision = other.m_subdivision;
		m_width = other.m_width;
		m_height = other.m_height;
		translation = other.translation;
		scale = other.scale;
		m_mass = other.m_mass;
		m_rigidity = other.m_rigidity;
		m_viscosity = other.m_viscosity;
		m_materialName = other.m_materialName;
		m_autoCollisionDistance = other.m_autoCollisionDistance;
		m_computeAutoCollision = other.m_computeAutoCollision;
		m_autoCollisionRigidity = other.m_autoCollisionRigidity;
		m_autoCollisionViscosity = other.m_autoCollisionViscosity;
		m_castShadows = other.m_castShadows;

		modelMatrix = other.modelMatrix;

		//don't forget to change the origin to have the right pivot rotation
		origin = other.origin;
		m_mesh.origin = other.m_mesh.origin;

		m_mesh.topRight = other.m_mesh.topRight;
		m_mesh.bottomLeft = other.m_mesh.bottomLeft;

		m_mesh.vertices.clear();
		m_mesh.normals.clear();
		m_mesh.uvs.clear();
		m_mesh.triangleIndex.clear();
		m_mesh.tangents.clear();

		localPointPositions.clear();
		pointContainer.clear();
		linkShape.clear();
		linkShearing.clear();
		linkBlending.clear();

		generatePoints();
		generateMesh();

		m_mesh.initGl();

		initialyzePhysic();

		//cover the mesh with collider : 
		if (m_entity != nullptr)
		{
			auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
			if (collider != nullptr)
			{
				collider->coverMesh(m_mesh);
				collider->setOffsetScale(glm::vec3(1.f, 1.f, 2.f));
			}
		}

		return *this;
	}

	Flag::~Flag()
	{
	}

	void Flag::generatePoints()
	{
		float paddingX = m_width / (float)(m_subdivision - 1);
		float paddingY = m_height / (float)(m_subdivision - 1);

		for (int j = 0; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{
				//physic elements : 
				localPointPositions.push_back(glm::vec3(i*paddingX, j*paddingY, 0.f));
				pointContainer.push_back(Point(glm::vec3(i*paddingX, j*paddingY, 0.f), glm::vec3(0, 0, 0), 0.f));
			}
		}
	}

	void Flag::regenerateFlag()
	{
		modelMatrix = glm::mat4(1);

		//don't forget to change the origin to have the right pivot rotation
		origin = glm::vec3(-0.5f, -0.5f, 0.f);
		m_mesh.origin = glm::vec3(-0.5f, -0.5f, 0.f);

		m_mesh.topRight = glm::vec3(m_width, m_height, 0.f);
		m_mesh.bottomLeft = glm::vec3(0.f, 0.f, 0.f);

		m_mesh.vertices.clear();
		m_mesh.normals.clear();
		m_mesh.uvs.clear();
		m_mesh.triangleIndex.clear();
		m_mesh.tangents.clear();

		localPointPositions.clear();
		pointContainer.clear();
		linkShape.clear();
		linkShearing.clear();
		linkBlending.clear();

		generatePoints();
		generateMesh();

		m_mesh.updateAllVBOs();

		initialyzePhysic();

		//cover the mesh with collider : 
		if (m_entity != nullptr)
		{
			auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
			if (collider != nullptr)
			{
				collider->coverMesh(m_mesh);
				collider->setOffsetScale(glm::vec3(1.f, 1.f, 2.f));
			}
		}

		//applied old transforms to the vertices :
		for (int i = 0; i < pointContainer.size(); i++)
		{
			pointContainer[i].position = glm::vec3(glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale) *  glm::vec4(localPointPositions[i], 1.f));
			pointContainer[i].setVitesse(glm::vec3(0, 0, 0));
			pointContainer[i].setForce(glm::vec3(0, 0, 0));
		}

		synchronizeVisual();
	}

	void Flag::generateMesh()
	{
		float paddingX = m_width / (float)(m_subdivision - 1);
		float paddingY = m_height / (float)(m_subdivision - 1);

		int lineCount = (m_subdivision - 1);
		int rowCount = (m_subdivision - 1);
		//m_mesh.triangleCount = (m_subdivision - 1) * (m_subdivision - 1) * 2 + 1;
		m_mesh.totalTriangleCount = (m_subdivision - 1) * (m_subdivision - 1) * 2 + 1;

		// face 1 : 
		for (int j = 0; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{

				//visual elements : 
				m_mesh.vertices.push_back(i*paddingX);
				m_mesh.vertices.push_back(j*paddingY);
				m_mesh.vertices.push_back(0.1f);

				m_mesh.normals.push_back(0);
				m_mesh.normals.push_back(0);
				m_mesh.normals.push_back(1);

				m_mesh.tangents.push_back(1);
				m_mesh.tangents.push_back(0);
				m_mesh.tangents.push_back(0);

				m_mesh.uvs.push_back(i / (float)(m_subdivision - 1));
				m_mesh.uvs.push_back(j / (float)(m_subdivision - 1));
			}
		}

		// face 2 : 
		for (int j = 0; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{

				//visual elements : 
				m_mesh.vertices.push_back(i*paddingX);
				m_mesh.vertices.push_back(j*paddingY);
				m_mesh.vertices.push_back(-0.1f);

				m_mesh.normals.push_back(0);
				m_mesh.normals.push_back(0);
				m_mesh.normals.push_back(-1);

				m_mesh.tangents.push_back(1);
				m_mesh.tangents.push_back(0);
				m_mesh.tangents.push_back(0);

				m_mesh.uvs.push_back(i / (float)(m_subdivision - 1));
				m_mesh.uvs.push_back(j / (float)(m_subdivision - 1));
			}
		}

		// face 1 : 
		int k = 0;
		for (int i = 0; i < m_mesh.totalTriangleCount; i++)
		{
			if (i % 2 == 0)
			{
				m_mesh.triangleIndex.push_back(k + 0);
				m_mesh.triangleIndex.push_back(k + 1);
				m_mesh.triangleIndex.push_back(k + m_subdivision);
			}
			else
			{
				m_mesh.triangleIndex.push_back(k + 1);
				m_mesh.triangleIndex.push_back(k + m_subdivision + 1);
				m_mesh.triangleIndex.push_back(k + m_subdivision);
			}

			if (i % 2 == 0 && i != 0)
				k++;

			if ((k + 1) % (m_subdivision) == 0 && i != 0)
			{
				k++;
			}
		}

		k += m_subdivision;

		// face 2 : 
		for (int i = 0; i < m_mesh.totalTriangleCount; i++)
		{
			if (i % 2 == 0)
			{
				m_mesh.triangleIndex.push_back(k + 0);
				m_mesh.triangleIndex.push_back(k + m_subdivision);
				m_mesh.triangleIndex.push_back(k + 1);
			}
			else
			{
				m_mesh.triangleIndex.push_back(k + 1);
				m_mesh.triangleIndex.push_back(k + m_subdivision);
				m_mesh.triangleIndex.push_back(k + m_subdivision + 1);
			}

			if (i % 2 == 0 && i != 0)
				k++;

			if ((k + 1) % (m_subdivision) == 0 && i != 0)
			{
				k++;
			}
		}

		m_mesh.totalTriangleCount *= 2;

	}

	void Flag::updateNormals()
	{
		int verticesPerFace = m_subdivision * m_subdivision;

		glm::vec3 u(1, 0, 0);
		glm::vec3 v(0, 0, 1);
		glm::vec3 normal(0, 0, 0);
		glm::vec3 tangent(0, 0, 0);

		for (int j = 0, k = 0; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++, k += 3)
			{

				if (j - 1 >= 0 && i - 1 >= 0)
				{
					u = vertexFrom3Floats(m_mesh.vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_mesh.vertices, (i - 1) + j * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (i - 1 >= 0 && j + 1 < (m_subdivision))
				{
					u = vertexFrom3Floats(m_mesh.vertices, (i - 1) + j * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_mesh.vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (j + 1 < (m_subdivision) && i + 1 < (m_subdivision))
				{
					u = vertexFrom3Floats(m_mesh.vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_mesh.vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (i + 1 < (m_subdivision) && j - 1 >= 0)
				{
					u = vertexFrom3Floats(m_mesh.vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_mesh.vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_mesh.vertices, i + j * m_subdivision);

					normal += glm::normalize(glm::cross(u, v));
				}

				normal = glm::normalize(normal);


				//face 1 
				m_mesh.normals[k] = normal.x;
				m_mesh.normals[k + 1] = normal.y;
				m_mesh.normals[k + 2] = normal.z;

				//face 2 
				m_mesh.normals[k + verticesPerFace * 3] = -normal.x;
				m_mesh.normals[k + 1 + verticesPerFace * 3] = -normal.y;
				m_mesh.normals[k + 2 + verticesPerFace * 3] = -normal.z;

				tangent = glm::normalize(glm::cross(normal, u));

				//face 1 
				m_mesh.tangents[k] = tangent.x;
				m_mesh.tangents[k + 1] = tangent.y;
				m_mesh.tangents[k + 2] = tangent.z;

				//face 2 
				m_mesh.tangents[k + verticesPerFace * 3] = -tangent.x;
				m_mesh.tangents[k + 1 + verticesPerFace * 3] = -tangent.y;
				m_mesh.tangents[k + 2 + verticesPerFace * 3] = -tangent.z;
			}
		}


		m_mesh.updateVBO(Mesh::Vbo_types::NORMALS);
		m_mesh.updateVBO(Mesh::Vbo_types::TANGENTS);
	}

	void Flag::initialyzePhysic()
	{
		float l = 0.f;

		//intialyze physic links : 
		for (int j = 0; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{
				Point* current = &pointContainer[idx2DToIdx1D(i, j, m_subdivision)];

				//shape links

				if (i + 1 < m_subdivision)
				{
					Point* right = &pointContainer[idx2DToIdx1D(i + 1, j, m_subdivision)];
					l = glm::distance(right->position, current->position);
					linkShape.push_back(Link(current, right, m_rigidity, m_viscosity, l)); //right link
				}

				if (j + 1 < m_subdivision)
				{
					Point* up = &pointContainer[idx2DToIdx1D(i, j + 1, m_subdivision)];
					l = glm::distance(up->position, current->position);
					linkShape.push_back(Link(up, current, m_rigidity, m_viscosity, l)); //up link
				}

				//shearing links

				if (j + 1 < m_subdivision && i + 1 < m_subdivision)
				{
					Point* rightUp = &pointContainer[idx2DToIdx1D(i + 1, j + 1, m_subdivision)];
					l = glm::distance(rightUp->position, current->position);
					linkShearing.push_back(Link(current, rightUp, m_rigidity, m_viscosity, l)); //right up link
				}
				if (j - 1 > 0 && i + 1 < m_subdivision)
				{
					Point* rightDown = &pointContainer[idx2DToIdx1D(i + 1, j - 1, m_subdivision)];
					l = glm::distance(rightDown->position, current->position);
					linkShearing.push_back(Link(current, rightDown, m_rigidity, m_viscosity, l)); //right down link
				}

				//blending links : 

				if (i + 2 < m_subdivision)
				{
					Point* right2 = &pointContainer[idx2DToIdx1D(i + 2, j, m_subdivision)];
					l = glm::distance(right2->position, current->position);
					linkBlending.push_back(Link(current, right2, m_rigidity, m_viscosity, l)); //right link
				}

				if (j + 2 < m_subdivision)
				{
					Point* up2 = &pointContainer[idx2DToIdx1D(i, j + 2, m_subdivision)];
					l = glm::distance(up2->position, current->position);
					linkBlending.push_back(Link(current, up2, m_rigidity, m_viscosity, l)); //up link
				}
			}
		}



		//set masses : 
		for (int i = 0; i < m_subdivision; i++)
		{
			pointContainer[i].masse = 0.f;
		}

		for (int j = 1; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{
				pointContainer[idx2DToIdx1D(i, j, m_subdivision)].masse = m_mass / (float)(m_subdivision * m_subdivision);
			}
		}
	}

	void Flag::updatePhysic()
	{
		//update rigidity and viscosity of links : 
		for (int i = 0; i < linkShape.size(); i++)
		{
			linkShape[i].k = m_rigidity;
			linkShape[i].z = m_viscosity;
		}
		for (int i = 0; i < linkBlending.size(); i++)
		{
			linkBlending[i].k = m_rigidity;
			linkBlending[i].z = m_viscosity;
		}
		for (int i = 0; i < linkShearing.size(); i++)
		{
			linkShearing[i].k = m_rigidity;
			linkShearing[i].z = m_viscosity;
		}

		//update masses : 
		for (int i = 0; i < m_subdivision; i++)
		{
			pointContainer[i].masse = 0.f;
		}

		for (int j = 1; j < m_subdivision; j++)
		{
			for (int i = 0; i < m_subdivision; i++)
			{
				pointContainer[idx2DToIdx1D(i, j, m_subdivision)].masse = m_mass / (float)(m_subdivision * m_subdivision);
			}
		}
	}

	void Flag::restartSimulation()
	{


		origin = glm::vec3(-0.5f, -0.5f, 0.f);
		m_mesh.origin = glm::vec3(-0.5f, -0.5f, 0.f);

		m_mesh.topRight = glm::vec3(m_width, m_height, 0.f);
		m_mesh.bottomLeft = glm::vec3(0.f, 0.f, 0.f);
		
		m_mesh.vertices.clear();
		m_mesh.normals.clear();
		m_mesh.uvs.clear();
		m_mesh.triangleIndex.clear();
		m_mesh.tangents.clear();

		generateMesh();
	
		for (int i = 0; i < pointContainer.size(); i++)
		{
			pointContainer[i].position = glm::vec3(glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale) *  glm::vec4(localPointPositions[i], 1.f));
			pointContainer[i].setVitesse(glm::vec3(0, 0, 0));
			pointContainer[i].setForce(glm::vec3(0, 0, 0));
		}

		synchronizeVisual();

	}

	void Flag::save(Json::Value & rootComponent) const
	{
		Component::save(rootComponent);

		rootComponent["origin"] = toJsonValue(origin);
		rootComponent["translation"] = toJsonValue(translation);
		rootComponent["scale"] = toJsonValue(scale);
		rootComponent["rotation"] = toJsonValue(rotation);
		rootComponent["modelMatrix"] = toJsonValue(modelMatrix);
		
		rootComponent["materialName"] = m_materialName;
		m_material.save(rootComponent["material"]);

		rootComponent["width"] = m_width;
		rootComponent["height"] = m_height;
		rootComponent["subdivision"] = m_subdivision;

		rootComponent["mass"] = m_mass;
		rootComponent["viscosity"] = m_viscosity;
		rootComponent["rigidity"] = m_rigidity;
		rootComponent["autoCollisionDistance"] = m_autoCollisionDistance;
		rootComponent["computeAutoCollision"] = m_computeAutoCollision;
		rootComponent["autoCollisionViscosity"] = m_autoCollisionViscosity;
		rootComponent["autoCollisionRigidity"] = m_autoCollisionRigidity;

	}

	void Flag::load(const Json::Value & rootComponent)
	{
		Component::load(rootComponent);

		origin = fromJsonValue<glm::vec3>(rootComponent["origin"], glm::vec3());
		translation = fromJsonValue<glm::vec3>(rootComponent["translation"], glm::vec3());
		scale = fromJsonValue<glm::vec3>(rootComponent["scale"], glm::vec3());
		rotation = fromJsonValue<glm::quat>(rootComponent["rotation"], glm::quat());
		modelMatrix = fromJsonValue<glm::mat4>(rootComponent["modelMatrix"], glm::mat4());

		m_materialName = rootComponent.get("materialName", "default").asString(); //TODO : remove ? 
		m_material.load(rootComponent["material"]);

		m_width = rootComponent.get("width", 10).asFloat();
		m_height = rootComponent.get("height", 10).asFloat();
		m_subdivision = rootComponent.get("subdivision", 10).asInt();

		m_mass = rootComponent.get("mass", 0.1).asFloat();
		m_viscosity = rootComponent.get("viscosity", 0.01).asFloat();
		m_rigidity = rootComponent.get("rigidity", 0.001).asFloat();
		m_autoCollisionDistance = rootComponent.get("autoCollisionDistance", 0.01f).asFloat();
		m_computeAutoCollision = rootComponent.get("computeAutoCollision", false).asBool();
		m_autoCollisionRigidity = rootComponent.get("autoCollisionRigidity", 0.01f).asFloat();
		m_autoCollisionViscosity = rootComponent.get("autoCollisionViscosity", 0.01f).asFloat();

		//no need to save physic infos because we rebuild it in initialisation
		regenerateFlag();
	}

	void Flag::computeAutoCollision()
	{
		glm::vec3 bottomLeft = m_mesh.bottomLeft;
		glm::vec3 topRight = m_mesh.topRight;
		glm::vec3 center = bottomLeft + (topRight - bottomLeft)*0.5f;
		float halfSize = std::max(topRight.x - bottomLeft.x, std::max(topRight.y - bottomLeft.y, topRight.z - bottomLeft.z) )*0.5f;
		Octree<Point, glm::vec3> octree(center, halfSize, 2);
		for (int i = 0; i < pointContainer.size(); i++)
			octree.add(&pointContainer[i], pointContainer[i].position);

		//draw octree : 
		std::vector<glm::vec3> octreeCenters;
		std::vector<float> octreeHalfSizes;
		octree.getAllCenterAndSize(octreeCenters, octreeHalfSizes);
		OctreeDrawer::get().addDrawItems(octreeCenters, octreeHalfSizes);

		float maxRadius = std::max(m_width, m_height)/ (float)m_subdivision;
		maxRadius *= 2.f;

		std::vector<Point*> neighborPoints;
		for (int i = 0; i < pointContainer.size(); i++)
		{
			octree.findNeighborsContained(pointContainer[i].position, maxRadius, neighborPoints);
			for (int j = 0; j < neighborPoints.size(); j++)
			{
				glm::vec3 pointToNeightbor = neighborPoints[j]->position - pointContainer[i].position;
				float distancePointToNeightbor = glm::length(pointToNeightbor);
				if (distancePointToNeightbor < m_autoCollisionDistance && (neighborPoints[j] != &pointContainer[i]))
				{
					glm::vec3 pushBackForce = glm::normalize(pointToNeightbor) * m_autoCollisionRigidity*(1.f - distancePointToNeightbor / m_autoCollisionDistance)*(1.f - distancePointToNeightbor / m_autoCollisionDistance);
					glm::vec3 breakForce = m_autoCollisionViscosity*(neighborPoints[j]->vitesse - pointContainer[i].vitesse);

					neighborPoints[j]->setForce(pushBackForce - breakForce);
					pointContainer[i].setForce(-pushBackForce + breakForce);
				}
			}

			neighborPoints.clear();
			octree.remove(&pointContainer[i], pointContainer[i].position);
		}
	}

	const IDrawable & Flag::getDrawable(int drawableIndex) const
	{
		return *this;
	}

	const MaterialInstance & Flag::getDrawableMaterial(int drawableIndex) const
	{
		return *m_material.get();
	}

	const int Flag::getDrawableCount() const
	{
		return 1;
	}

	Component * Flag::getAsComponent()
	{
		return this;
	}

	const AABB & Flag::getVisualBoundingBox() const
	{
		return m_mesh.getLocalAABB();
	}

	void Flag::draw() const
	{
		m_mesh.draw();
	}

	void Physic::Flag::computeGlobalBreak(float deltaTime, Point* point)
	{
		point->force -= m_viscosity*(point->vitesse);
	}

	void Physic::Flag::update(float deltaTime)
	{
		//points : 
		for (int i = 0; i < pointContainer.size(); i++)
			computePoints(deltaTime, &pointContainer[i]);

		
		//shape : 
		for (int i = 0; i < linkShape.size(); i++)
			computeLinks(deltaTime, &linkShape[i]);

		//shearing :
		for (int i = 0; i < linkShearing.size(); i++)
			computeLinks(deltaTime, &linkShearing[i]);

		//blending : 
		for (int i = 0; i < linkBlending.size(); i++)
			computeLinks(deltaTime, &linkBlending[i]);
		
		if(m_computeAutoCollision)
			computeAutoCollision();

		//for (int i = 0; i < pointContainer.size(); i++)
		//	computeGlobalBreak(deltaTime, &pointContainer[i]);

		synchronizeVisual();
	}

	void Flag::synchronizeVisual()
	{
		
		glm::vec3 min = pointContainer[0].position;
		glm::vec3 max = pointContainer[0].position;

		int verticePerFace = m_subdivision * m_subdivision;

		for (int i = 0, j = 0; i < pointContainer.size(); i++, j+=3)
		{
			m_mesh.vertices[j] = pointContainer[i].position.x + m_mesh.normals[j]*0.01f;
			m_mesh.vertices[j+1] = pointContainer[i].position.y + m_mesh.normals[j + 1] * 0.01f;
			m_mesh.vertices[j+2] = pointContainer[i].position.z + m_mesh.normals[j + 2] * 0.01f;

			m_mesh.vertices[j + verticePerFace*3] = pointContainer[i].position.x + m_mesh.normals[j + verticePerFace*3] * 0.01f;
			m_mesh.vertices[j + 1 + verticePerFace*3] = pointContainer[i].position.y + m_mesh.normals[j + 1 + verticePerFace*3] * 0.01f;
			m_mesh.vertices[j + 2 + verticePerFace*3] = pointContainer[i].position.z + m_mesh.normals[j + 2 + verticePerFace*3] * 0.01f;

			//calculate bounds : 
			if (pointContainer[i].position.x < min.x)
				min.x = pointContainer[i].position.x;
			if (pointContainer[i].position.y < min.y)
				min.y = pointContainer[i].position.y;
			if (pointContainer[i].position.z < min.z)
				min.z = pointContainer[i].position.z;

			if (pointContainer[i].position.x > max.x)
				max.x = pointContainer[i].position.x;
			if (pointContainer[i].position.y > max.y)
				max.y = pointContainer[i].position.y;
			if (pointContainer[i].position.z > max.z)
				max.z = pointContainer[i].position.z;
		}

		m_mesh.updateVBO(Mesh::VERTICES);
	
		updateNormals(); 
		
		//update bounds : 
		m_mesh.bottomLeft = min;
		m_mesh.topRight = max;
		if (m_entity != nullptr)
		{
			auto collider = static_cast<BoxCollider*>(m_entity->getComponent(Component::ComponentType::COLLIDER));
			if (collider != nullptr)
			{
				//trick to remove scale and rotation from collider, because vertices are manually moved on scene and we want the collider to fit to the flag shape
				collider->scale = glm::vec3(1, 1, 1); 
				collider->rotation = glm::quat(0, 0, 0, 0);
				collider->translation = glm::vec3(0, 0, 0);
				collider->coverMesh(m_mesh);
			}
		}
	}

	void Flag::computeLinks(float deltaTime, Link* link)
	{
		float d = glm::distance(link->M1->position, link->M2->position);
		if (d < 0.00000001f)
			return;

		float f = -link->k * (1.f - d / link->l);
		if (std::abs(f) < 0.00000001f)
			return;

		glm::vec3 M1M2 = link->M2->position - link->M1->position;
		glm::normalize(M1M2);
		if (glm::length(M1M2) < 0.00000001f)
			return;

		//frein :
		glm::vec3 frein = link->z*(link->M2->vitesse - link->M1->vitesse);

		link->M1->force += (f * M1M2 + frein);
		link->M2->force += (-f * M1M2 - frein);

	}

	void Flag::computePoints(float deltaTime, Point* point)
	{
		if (point->masse < 0.00000001f)
			return;

		//leapfrog
		point->vitesse += (deltaTime / point->masse)*point->force;
		point->position += deltaTime*point->vitesse;
		point->force = glm::vec3(0, 0, 0);
	}

	void Physic::Flag::render(const glm::mat4& projection, const glm::mat4& view)
	{
		PRINT_WARNING("DEPRECATED[Physic::Flag::render]");

		glm::mat4 mvp = projection * view * modelMatrix;
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

		Material3DObject* castedMaterial = static_cast<Material3DObject*>(m_material.get());

		castedMaterial->use();
		//castedMaterial->setUniform_MVP(mvp);
		//castedMaterial->setUniform_normalMatrix(normalMatrix);
		castedMaterial->setUniformModelMatrix(modelMatrix);
		castedMaterial->setUniformViewMatrix(view);
		castedMaterial->setUniformProjectionMatrix(projection);



		m_mesh.draw();
	}

	void Flag::drawInInspector(Scene& scene)
	{
		if (ImGui::InputFloat("mass", &m_mass))
			updatePhysic();
		if(ImGui::InputFloat("viscosity", &m_viscosity))
			updatePhysic();
		if(ImGui::InputFloat("rigidity", &m_rigidity))
			updatePhysic();

		int tmpSub = m_subdivision;
		if (ImGui::InputInt("subdivision", &m_subdivision))
		{
			m_mass *= ((m_subdivision * m_subdivision) / (float)(tmpSub*tmpSub)); // change mass because we add matter, otherwise the system isn't stable
			regenerateFlag();
		}

		if (ImGui::Button("restart simulation"))
			restartSimulation();

		if (ImGui::RadioButton("computeAutoCollision", m_computeAutoCollision))
			m_computeAutoCollision = !m_computeAutoCollision;

		ImGui::InputFloat("autoCollisionDistance", &m_autoCollisionDistance);
		ImGui::InputFloat("autoCollisionRigidity", &m_autoCollisionRigidity);
		ImGui::InputFloat("autoCollisionViscosity", &m_autoCollisionViscosity);

		//char tmpMaterialName[100];
		//m_materialName.copy(tmpMaterialName, m_materialName.size());
		//tmpMaterialName[m_materialName.size()] = '\0';
		//%NOCOMMIT%
		//if (ImGui::InputText("materialName", tmpMaterialName, 20))
		//{
		//	m_materialName = tmpMaterialName;

		//	if (getMaterialFactory().contains(m_materialName))
		//	{
		//		m_material = getMaterialFactory().getDefault(m_materialName);
		//	}
		//}

		//EditorGUI::ResourceField(m_material, "materialName", tmpMaterialName, 100);
		EditorGUI::ResourceField<MaterialInstance>("materialName", m_material);

		//m_material->drawUI();
	}

	void Flag::drawInInspector(Scene& scene, const std::vector<Component*>& components)
	{
		float _mass = m_mass;
		float _viscosity = m_viscosity;
		float _rigidity = m_rigidity;
		if (ImGui::InputFloat("mass", &_mass))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_mass = _mass;
				updatePhysic();
			}
		}
		if (ImGui::InputFloat("viscosity", &_viscosity))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_viscosity = _viscosity;
				updatePhysic();
			}
		}
		if (ImGui::InputFloat("rigidity", &_rigidity))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_rigidity = _rigidity;
				updatePhysic();
			}
		}

		int _subdivision = m_subdivision;
		int tmpSub = m_subdivision;
		if (ImGui::InputInt("subdivision", &_subdivision))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_subdivision = _subdivision;
				castedComponent->m_mass *= ((castedComponent->m_subdivision * castedComponent->m_subdivision) / (float)(tmpSub*tmpSub)); // change mass because we add matter, otherwise the system isn't stable
				castedComponent->regenerateFlag();
			}
		}

		if (ImGui::Button("restart simulation"))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->restartSimulation();
			}
		}

		bool _computeAutoCollision = m_computeAutoCollision;
		if (ImGui::RadioButton("computeAutoCollision", _computeAutoCollision))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_computeAutoCollision = !_computeAutoCollision;
			}
		}

		if(ImGui::InputFloat("autoCollisionDistance", &m_autoCollisionDistance))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_autoCollisionDistance = m_autoCollisionDistance;
			}
		}
		if (ImGui::InputFloat("autoCollisionRigidity", &m_autoCollisionRigidity))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_autoCollisionRigidity = m_autoCollisionRigidity;
			}
		}
		if(ImGui::InputFloat("autoCollisionViscosity", &m_autoCollisionViscosity))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_autoCollisionViscosity = m_autoCollisionViscosity;
			}
		}

		if (EditorGUI::ResourceField<MaterialInstance>("materialName", m_material))
		{
			for (auto component : components)
			{
				Flag* castedComponent = static_cast<Flag*>(component);
				castedComponent->m_material = m_material;
			}
		}
	}


	void Flag::applyForce(const glm::vec3 & force)
	{
		for (int i = 0; i < pointContainer.size(); i++)
			pointContainer[i].force += force;
	}

	void Flag::applyGravity(const glm::vec3 & gravity)
	{
		for (int i = 0; i < pointContainer.size(); i++)
			pointContainer[i].force += (gravity * pointContainer[i].masse); // weight = m * g 
	}

	Mesh & Flag::getMesh() 
	{
		return m_mesh;
	}

	void Flag::applyTransform(const glm::vec3& _translation, const glm::vec3& _scale, const glm::quat& _rotation)
	{
		modelMatrix = glm::translate(glm::mat4(1), _translation) * glm::mat4_cast(_rotation) * glm::scale(glm::mat4(1), _scale);

		for (int i = 0; i < m_subdivision; i++)
		{
			pointContainer[i].position = glm::vec3(glm::translate(glm::mat4(1), _translation) * glm::mat4_cast(_rotation) *  glm::vec4(localPointPositions[i], 1.f));
		}

		for (int i = 0; i < pointContainer.size(); i++)
		{
			pointContainer[i].position = glm::vec3(glm::scale(glm::mat4(1), 1.f/scale) * glm::vec4(pointContainer[i].position, 1.f)); // inverse previous scale
			pointContainer[i].position = glm::vec3( glm::scale(glm::mat4(1), _scale) * glm::vec4(pointContainer[i].position, 1.f));
		}

		modelMatrix = glm::mat4(1);//glm::translate(glm::mat4(1), -translation);

		translation = _translation;
		rotation = _rotation;
		if (_scale != scale)
			setDimensions(_scale.x, _scale.y);
		scale = _scale;

		synchronizeVisual();
	}

	glm::vec3 Flag::getOrigin() const
	{
		return origin;
	}

	const glm::mat4 & Flag::getModelMatrix() const
	{
		return modelMatrix;
	}

	bool Flag::castShadows() const
	{
		return m_castShadows;
	}

	void Flag::setExternalsOf(const MaterialLit & material, const glm::mat4 & projection, const glm::mat4 & view, int* texId) const
	{
		// Transform matrices
		material.setUniformModelMatrix(getModelMatrix());
		material.setUniformViewMatrix(view);
		material.setUniformProjectionMatrix(projection);
		material.setUniformUseSkeleton(false);
	}

	float Flag::getMass() const
	{
		return m_mass;
	}

	float Flag::getRigidity() const
	{
		return m_rigidity;
	}

	float Flag::getViscosity() const
	{
		return m_viscosity;
	}

	void Flag::setMass(float mass)
	{
		m_mass = mass;
	}

	void Flag::setRigidity(float rigidity)
	{
		m_rigidity = rigidity;
	}

	void Flag::setViscosity(float viscosity)
	{
		m_viscosity = viscosity;
	}

	void Flag::setDimensions(float width, float height)
	{
		m_width = width;
		m_height = height;
		regenerateFlag();
	}

	void Flag::setSubdivision(int subdivision)
	{
		m_subdivision = subdivision;
	}

	int Flag::getSubdivision() const
	{
		return m_subdivision;
	}


	void Flag::onAfterComponentAddedToScene(Scene & scene)
	{
		//Add this components to renderables :
		IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
		if (asRenderable->getDrawableCount() > 0)
			scene.addToRenderables(this);
	}

	void Flag::onBeforeComponentErasedFromScene(Scene & scene)
	{
		//Remove this components from renderables :
		IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
		if (asRenderable->getDrawableCount() > 0)
			scene.removeFromRenderables(this);
	}

}
