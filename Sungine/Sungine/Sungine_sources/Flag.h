#pragma once

#include <vector>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "Point.h"
#include "Link.h"

#include "Utils.h"
#include "Mesh.h"
#include "Materials.h"
#include "Octree.h"
#include "Component.h"
#include "BatchableWith.h"
#include "IRenderableComponent.h"
#include "IDrawable.h"

namespace Physic {

	class Flag : public Component, public IBatchableWith<MaterialLit>, public IRenderableComponent
	{
		COMPONENT_IMPLEMENTATION_HEADER(Flag)

		CLASS((Flag, Component),
		((PRIVATE)
			// Transform override
			(glm::vec3	, origin					)
			(glm::vec3	, translation				)
			(glm::vec3	, scale						)
			(glm::quat	, rotation					)
			(glm::mat4	, modelMatrix				)

			// Flag parameters
			(bool		, m_castShadows				)

			(float		, m_width					)
			(float		, m_height					)
			(int		, m_subdivision				)
			
			(float		, m_mass					)
			(float		, m_viscosity				)
			(float		, m_rigidity				)
			
			//for auto collisions : 
			(float		, m_autoCollisionDistance	)
			(bool		, m_computeAutoCollision	)
			(float		, m_autoCollisionRigidity	)
			(float		, m_autoCollisionViscosity	)

			//flag material
			(AssetHandle<MaterialInstance>, m_material)
			)
		)

	private:
		// internal infos
		std::vector<glm::vec3> localPointPositions;
		std::vector<Point> pointContainer;
		//maillages structurel (d4)
		std::vector<Link> linkShape;
		//maillage diagonal (d8\d4)
		std::vector<Link> linkShearing;
		//maillage pont (tout les deux points)
		std::vector<Link> linkBlending;

		// Generated mesh
		Mesh m_mesh;

		// Flag material
		std::string m_materialName; // ???

	public:
		Flag();
		Flag(ResourcePtr<MaterialInstance> material, int subdivision = 10, float width = 10.f, float height = 10.f);
		Flag(const Flag& other);
		Flag& operator=(const Flag& other);
		~Flag();

		//function to call each frame, with deltaTime = frame duration or fixe duration
		void update(float deltaTime);
		
		//render the flag with the camera projection and view
		void render(const glm::mat4& projection, const glm::mat4& view);

		//draw the UI for the flag
		virtual void drawInInspector(Scene& scene) override;
		virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

		//add a force to each point the flag
		void applyForce(const glm::vec3& force);
		//add gravity to each point of the flag
		void applyGravity(const glm::vec3& gravity);

		//return the flag mesh
		Mesh& getMesh();

		//apply a transform operation to the flag, modelMatrix only retain the translation. Rotation and scale are applied locally on each point of the flag
		virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;

		//Return the origin of the flag, which is normaly also the origin of the flag mesh.
		glm::vec3 getOrigin() const;

		float getMass() const;
		float getRigidity() const;
		float getViscosity() const;
		void setMass(float mass);
		void setRigidity(float rigidity);
		void setViscosity(float viscosity);
		void setDimensions(float width, float height);

		void setSubdivision(int subdivision);
		int getSubdivision() const;

		void updatePhysic();

		void restartSimulation();

		virtual void save(Json::Value& rootComponent) const override;
		virtual void load(const Json::Value& rootComponent) override;

		// Herited from IRenderableComponent
		virtual const IDrawable & getDrawable(int drawableIndex) const override;
		virtual const MaterialInstance & getDrawableMaterial(int drawableIndex) const override;
		virtual const int getDrawableCount() const override;
		virtual Component* getAsComponent() override;

		// Herited from IDrawable
		virtual const AABB & getVisualBoundingBox() const override;
		virtual void draw() const override;
		virtual const glm::mat4& getModelMatrix() const override;
		virtual bool castShadows() const override;

		virtual void setExternalsOf(const MaterialLit& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const override;

	private : 
		//completly free memory allocate for the flag and then reconstruct the flag
		void regenerateFlag();
		//simply generate the model, don't destroy or allocate memory, we have to do it manually before and after calling this function
		void generateMesh();
		void generatePoints(); 
		//update all normals such that they follow the shape
		void updateNormals();
		//initialyze and place the physic points and links
		void initialyzePhysic();
		//function call at each update, it will update vertices, normals (calling updateNormals())
		void synchronizeVisual(); // update the mesh to follow the physic points

		//apply physic simulation on links
		void computeLinks(float deltaTime, Link* link);
		//apply physic simulation on points
		void computePoints(float deltaTime, Point* point);
		void computeGlobalBreak(float deltaTime, Point* point);

		void computeAutoCollision();

		virtual void onAfterComponentAddedToScene(Scene & scene) override;
		virtual void onBeforeComponentErasedFromScene(Scene & scene) override;
};

}

REGISTER_CLASS(Physic::Flag)

