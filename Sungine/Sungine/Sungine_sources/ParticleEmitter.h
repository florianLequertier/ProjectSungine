#pragma once

#include "Mesh.h"
#include "Materials.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/gtc/random.hpp"

#include "Component.h"
#include "Materials.h"

#include "jsoncpp/json/json.h"
#include "SerializeUtils.h"
#include "ResourcePointer.h"
#include "IRenderableComponent.h"
#include "IDrawable.h"

namespace Physic{

	class ParticleEmitter : public Component, public IRenderableComponent, public IBatchableWith<MaterialParticlesCPU>
	{
		COMPONENT_IMPLEMENTATION_HEADER(ParticleEmitter)

		CLASS((ParticleEmitter, Component),
		((PRIVATE)
			//transform :
			(glm::vec3				, m_scale						)
			(glm::vec3				, m_translation					)
			(glm::quat				, m_rotation					)
			//parameters :
			(int					, m_maxParticleCount			)
			(int					, m_aliveParticlesCount			)
			(std::vector<float>		, m_sizeSteps_times				)
			(std::vector<glm::vec2> , m_sizeSteps_values			)
			(std::vector<float>		, m_colorSteps_times			)
			(std::vector<glm::vec4> , m_colorSteps_values			)
			(std::vector<float>		, m_forceSteps_times			)
			(std::vector<glm::vec3> , m_forceSteps_values			)
			(glm::vec2				, m_initialVelocityInterval		)
			(glm::vec2				, m_lifeTimeInterval			)
			(float					, m_particleCountBySecond		)
			(float					, m_spawnFragment				)
			(bool					, m_emitInShape					)
			(bool					, m_sortParticles				)
		)
		)

	public :
		enum VBO_TYPES { VERTICES = 0, NORMALS, UVS,  POSITIONS, COLORS, SIZES};
	private:

		//TODO : enlever a texture ? nouveau pipeline materiel
		ResourcePtr<Texture> m_particleTexture;
		std::string m_particleTextureName;

		//TODO : -> parameters
		//materials : 
		ResourcePtr<Material> m_materialParticules;
		//MaterialParticleSimulation* m_materialParticuleSimulation;

		//particles soa : 
		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_velocities;
		std::vector<glm::vec3> m_forces;
		std::vector<float> m_elapsedTimes;
		std::vector<float> m_lifeTimes;
		std::vector<glm::vec4> m_colors;
		std::vector<glm::vec2> m_sizes;
		std::vector<float> m_distanceToCamera;

		//model :
		int m_triangleCount;
		std::vector<unsigned int> m_triangleIndex;
		std::vector<float> m_uvs;
		std::vector<float> m_vertices;
		std::vector<float> m_normals;

		//opengl stuff : 
		GLuint m_vao;
		GLuint m_index;
		GLuint m_vboUvs;
		GLuint m_vboVertices;
		GLuint m_vboNormals;
		//instanced infos : 
		GLuint m_vboPositions;
		GLuint m_vboColors;
		GLuint m_vboSizes;

	public:
		ParticleEmitter();
		~ParticleEmitter();
		void initGl();
		void swapParticles(int a_idx, int b_idx);
		glm::vec3 getInternalParticleForce(float elapsedTime, float lifeTime, const glm::vec3& position);
		glm::vec2 getInternalParticleSize(float elapsedTime, float lifeTime, const glm::vec3& position);
		glm::vec4 getInternalParticleColor(float elapsedTime, float lifeTime, const glm::vec3& position);
		glm::vec3 getInitialVelocity() const;
		float getInitialLifeTime() const;
		void spawnParticles(int spawnCount);
		void update(float deltaTime, const glm::vec3& cameraPosition);
		void sortParticles();
		void render(const glm::mat4& projection, const glm::mat4& view);
		void updateVbos();
		void onChangeMaxParticleCount();
		const Texture& getParticleTexture() const;

		//TODO

		virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
		
		//draw the ui of particle emitter
		virtual void drawInInspector(Scene& scene) override;
		virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

		//herited from Component
		virtual void save(Json::Value& rootComponent) const override;
		virtual void load(const Json::Value& rootComponent) override;

		//herited from IRenderableComponent
		virtual const IDrawable & getDrawable(int drawableIndex) const override;
		virtual const Material & getDrawableMaterial(int drawableIndex) const override;
		virtual const int getDrawableCount() const override;
		virtual Component* getAsComponent() override;

		// Herited from IDrawable
		virtual const AABB & getVisualBoundingBox() const override;
		virtual void draw() const override;
		virtual const glm::mat4& getModelMatrix() const override;
		virtual bool castShadows() const override;
		virtual const Physic::ParticleEmitter* getAsParticleEmiterIfPossible() const override;

		virtual void setExternalsOf(const MaterialParticlesCPU& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const override;

		virtual void onAfterComponentAddedToScene(Scene & scene) override;
		virtual void onBeforeComponentErasedFromScene(Scene & scene) override;

	private:
		void sorting_quickSort(int begin, int end);
		int sorting_partition(int begin, int end);
};

}

REGISTER_CLASS(Physic::ParticleEmitter)

